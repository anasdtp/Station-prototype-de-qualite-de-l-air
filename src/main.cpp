#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include "Adafruit_PM25AQI.h"

#define PIN_NEOPIX 6     // Broche reliée à DIN du NeoPixel Ring
#define NPIX 12          // Nombre de LEDs sur l'anneau
#define SEALEVEL_HPA 1013.25  // Pression atmosphérique au niveau de la mer (à ajuster selon ta localisation)

// PMS5003 sensor configuration
#define PMS_RX_PIN 4     // Connect to TX of PMS5003
#define PMS_TX_PIN 5     // Connect to RX of PMS5003

Adafruit_BME280 bme;     // Objet BME280 (I2C)
Adafruit_NeoPixel strip(NPIX, PIN_NEOPIX, NEO_GRB + NEO_KHZ800);

// PMS5003 sensor objects
// SoftwareSerial Serial1(PMS_RX_PIN, PMS_TX_PIN);
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

// Structure pour stocker l'indice de qualité environnementale
struct EnvironmentalQuality {
  float temperature;      // Température en °C
  float airQuality;      // PM2.5 en μg/m³
  float correlation;     // Indice de corrélation (0-100)
  int ledCount;         // Nombre de LEDs à allumer (0-12)
  uint32_t color;       // Couleur des LEDs
};

// Function prototypes
EnvironmentalQuality calculateEnvironmentalCorrelation(float temp, float pm25, float pm10);
void showEnvironmentalStatus(EnvironmentalQuality env);
void showErrorPattern();
bool beginBME();
void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(50);  // Réduit la luminosité du ring
  for (int i = 0; i < NPIX; i++) strip.setPixelColor(i, strip.Color(255, 0, 0)); // Rouge erreur
  strip.show();             // Éteint toutes les LEDs
  delay(1000);

  // Initialisation du PMS5003
  Serial.println("=== DIAGNOSTIC PMS5003 ===");
  Serial.print("Initialisation de SoftwareSerial sur pins ");
  Serial.print(PMS_RX_PIN); Serial.print(" (RX) et "); Serial.print(PMS_TX_PIN); Serial.println(" (TX)");
  
  Serial1.begin(9600);
  Serial.println("SoftwareSerial initialisé à 9600 bauds");
  
  // Attendre que le capteur soit prêt
  Serial.println("Attente de stabilisation du capteur (3 secondes)...");
  delay(3000);
  
  // Test de disponibilité des données
  Serial.print("Données disponibles sur SoftwareSerial: ");
  Serial.println(Serial1.available());
  
  // Initialiser la bibliothèque Adafruit PM25 avec SoftwareSerial
  Serial.println("Tentative d'initialisation avec la bibliothèque Adafruit...");
  if (!aqi.begin_UART(&Serial1)) {
    Serial.println("ÉCHEC: Impossible de trouver le capteur PMS5003!");
    Serial.println("Vérifications à faire:");
    Serial.println("1. Vérifiez l'alimentation du capteur (5V/3.3V)");
    Serial.println("2. Vérifiez le câblage:");
    Serial.println("   - PMS5003 TX -> Arduino Pin 2");
    Serial.println("   - PMS5003 RX -> Arduino Pin 3");
    Serial.println("   - PMS5003 VCC -> 5V");
    Serial.println("   - PMS5003 GND -> GND");
    Serial.println("3. Le capteur peut mettre 30 secondes à s'initialiser après mise sous tension");
  } else {
    Serial.println("SUCCÈS: Capteur PMS5003 détecté et initialisé!");
  }

  if (!beginBME()) {
    Serial.println(F("BME280 non trouvé. Vérifie le câblage SDA/SCL + alimentation."));
    for (int i = 0; i < NPIX; i++) strip.setPixelColor(i, strip.Color(255, 0, 0)); // Rouge erreur
    strip.show();
    while (1);
  }
  Serial.println("BME280 détecté. Test en cours...");
  digitalWrite(LED_BUILTIN, HIGH); // Allume la LED intégrée pour indiquer le succès  
}

void loop() {
  // Lecture des données BME280
  float t = bme.readTemperature();        // Température (°C)
  float h = bme.readHumidity();           // Humidité (%)
  float p = bme.readPressure() / 100.0F;  // Pression (hPa)
  float alt = bme.readAltitude(SEALEVEL_HPA); // Altitude approx. (m)

  // Affichage des données BME280
  Serial.println();
  Serial.println("=== DONNÉES ENVIRONNEMENTALES ===");
  Serial.print("Temp: "); Serial.print(t, 1); Serial.print(" °C | ");
  Serial.print("Humidité: "); Serial.print(h, 1); Serial.print(" % | ");
  Serial.print("Pression: "); Serial.print(p, 1); Serial.print(" hPa | ");
  Serial.print("Alt: "); Serial.print(alt, 1); Serial.println(" m");

  // Lecture des données PMS5003 avec la bibliothèque Adafruit
  PM25_AQI_Data pmsData;
  bool pmsDataAvailable = aqi.read(&pmsData);
  
  if (pmsDataAvailable) {
    // Lecture réussie!
    Serial.println();
    Serial.println("=== QUALITÉ DE L'AIR PMS5003 ===");
    Serial.println("---------------------------------------");
    Serial.println("Concentrations Standard (μg/m³)");
    Serial.print("PM 1.0: "); Serial.print(pmsData.pm10_standard);
    Serial.print("\t\tPM 2.5: "); Serial.print(pmsData.pm25_standard);
    Serial.print("\t\tPM 10: "); Serial.println(pmsData.pm100_standard);
    Serial.println("---------------------------------------");
    Serial.println("Concentrations Environnementales (μg/m³)");
    Serial.print("PM 1.0: "); Serial.print(pmsData.pm10_env);
    Serial.print("\t\tPM 2.5: "); Serial.print(pmsData.pm25_env);
    Serial.print("\t\tPM 10: "); Serial.println(pmsData.pm100_env);
    Serial.println("---------------------------------------");
    Serial.println("Comptage des particules (par 0.1L d'air)");
    Serial.print("Particules > 0.3μm: "); Serial.println(pmsData.particles_03um);
    Serial.print("Particules > 0.5μm: "); Serial.println(pmsData.particles_05um);
    Serial.print("Particules > 1.0μm: "); Serial.println(pmsData.particles_10um);
    Serial.print("Particules > 2.5μm: "); Serial.println(pmsData.particles_25um);
    Serial.print("Particules > 5.0μm: "); Serial.println(pmsData.particles_50um);
    Serial.print("Particules > 10μm: "); Serial.println(pmsData.particles_100um);
    Serial.println("---------------------------------------");
    Serial.println("Indices de Qualité de l'Air (AQI)");
    Serial.print("AQI PM2.5 (US): "); Serial.print(pmsData.aqi_pm25_us);
    Serial.print("\t\tAQI PM10 (US): "); Serial.println(pmsData.aqi_pm100_us);
    Serial.println("---------------------------------------");
  } else {
    Serial.println("Attente des données PMS5003...");
  }

  // Calcul et affichage de la corrélation environnementale
  if (pmsDataAvailable) {
    // Calcule la corrélation entre température et qualité de l'air
    EnvironmentalQuality envStatus = calculateEnvironmentalCorrelation(t, pmsData.aqi_pm25_us, pmsData.aqi_pm100_us);
    
    // Affiche les informations sur l'affichage LED
    Serial.println();
    Serial.println("=== AFFICHAGE LED ENVIRONNEMENTAL ===");
    Serial.print("Température: "); Serial.print(envStatus.temperature, 1); Serial.print(" °C -> ");
    Serial.print(envStatus.ledCount); Serial.println(" LEDs allumées");
    Serial.print("PM2.5: "); Serial.print(envStatus.airQuality, 1); Serial.print(" μg/m³ -> ");
    
    // Interprétation de la couleur basée sur la qualité de l'air
    if (envStatus.airQuality <= 12.0) {
      Serial.println("Couleur: VERT (Excellent)");
    } else if (envStatus.airQuality <= 35.0) {
      Serial.println("Couleur: JAUNE-VERT (Bon)");
    } else if (envStatus.airQuality <= 55.0) {
      Serial.println("Couleur: JAUNE (Modéré)");
    } else if (envStatus.airQuality <= 150.0) {
      Serial.println("Couleur: ORANGE (Mauvais pour sensibles)");
    } else if (envStatus.airQuality <= 250.0) {
      Serial.println("Couleur: ROUGE (Mauvais)");
    } else {
      Serial.println("Couleur: VIOLET (Très mauvais)");
    }
    
    Serial.println("Logique: Nombre de LEDs = Température | Couleur = Qualité de l'air");
    
    // Affiche le statut sur les LEDs
    showEnvironmentalStatus(envStatus);
  } else {
    // Si pas de données PMS5003, utilise seulement la température
    EnvironmentalQuality tempOnly = calculateEnvironmentalCorrelation(t, 0, 0); // PM2.5 = 0 (optimal)
    Serial.println("Affichage basé uniquement sur la température (PMS5003 non disponible)");
    showEnvironmentalStatus(tempOnly);
  }

  delay(2000); // Pause 2 secondes entre chaque mesure
}

// Calcule la corrélation température-qualité de l'air
EnvironmentalQuality calculateEnvironmentalCorrelation(float temp, float aqi_pm25, float aqi_pm10) {
  EnvironmentalQuality result;
  result.temperature = temp;
  result.airQuality = max(aqi_pm25, aqi_pm10); // Utilise la valeur la plus élevée pour la qualité de l'air
  
  // Nombre de LEDs basé sur la température (0-35°C -> 0-12 LEDs)
  float tempConstrained = constrain(temp, 0.0, 35.0);
  result.ledCount = map((int)(tempConstrained * 10), 0, 350, 0, 12);

  result.correlation = 100 - (result.airQuality / 200) * 100;
  result.correlation = max(0, min(100, result.correlation)) ;

  // Si T < 5°C → Probabilité d’inversion → Pollution aggravée → -10 %
  // Si T > 30°C → Pollution photochimique → -7 %

  if (temp <= 5) {        // inversion thermique : pollution piégée
    result.correlation -= 10;
  } else if (temp >= 27) {     // forte chaleur : formation d'ozone
    result.correlation -= 7;
  }
  Serial.println("Corrélation Température-Qualité de l'air: " + String(result.correlation) + "%");

  return result;
}

// Affiche l'état environnemental sur les LEDs
void showEnvironmentalStatus(EnvironmentalQuality env) {

  int colors[6][3] = {
    {0, 255, 0},    // Vert (Excellent)
    {127, 255, 0},  // Jaune-Vert (Bon)
    {255, 255, 0},  // Jaune (Modéré)
    {255, 165, 0},  // Orange (Mauvais pour sensibles)
    {255, 0, 0},    // Rouge (Mauvais)
    {128, 0, 128}   // Violet (Très mauvais)
  };
  // Détermine la couleur basée sur la qualité de l'air
  Serial.println("Qualité de l'air: " + String((int)env.correlation) + " %");
  int color = map((int)env.correlation, 100, 0, 0, 5);
  Serial.println("Couleur sélectionnée: " + String(color));
  if (color > 5) { color = 5; }
  env.color = strip.Color(colors[color][0], colors[color][1], colors[color][2]);

  // Animation de transition
  strip.clear();
  
  // Effet de remplissage progressif
  for (int i = 0; i < env.ledCount; i++) {
    strip.setPixelColor(i, env.color);
    strip.show();
    delay(50); // Animation fluide
  }
  
  // Effet de pulsation pour indiquer la corrélation
  for (int pulse = 0; pulse < 3; pulse++) {
    // Diminuer la luminosité
    for (int brightness = 255; brightness >= 50; brightness -= 10) {
      uint32_t dimmedColor = strip.Color(
        (env.color >> 16 & 0xFF) * brightness / 255,
        (env.color >> 8 & 0xFF) * brightness / 255,
        (env.color & 0xFF) * brightness / 255
      );
      for (int i = 0; i < env.ledCount; i++) {
        strip.setPixelColor(i, dimmedColor);
      }
      strip.show();
      delay(20);
    }
    
    // Augmenter la luminosité
    for (int brightness = 50; brightness <= 255; brightness += 10) {
      uint32_t dimmedColor = strip.Color(
        (env.color >> 16 & 0xFF) * brightness / 255,
        (env.color >> 8 & 0xFF) * brightness / 255,
        (env.color & 0xFF) * brightness / 255
      );
      for (int i = 0; i < env.ledCount; i++) {
        strip.setPixelColor(i, dimmedColor);
      }
      strip.show();
      delay(20);
    }
  }
}

// Affiche un indicateur d'erreur sur les LEDs
void showErrorPattern() {
  for (int i = 0; i < NPIX; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0)); // Rouge pour erreur
  }
  strip.show();
}

bool beginBME() {
  // Essaie adresse 0x76 puis 0x77
  return bme.begin(0x76) || bme.begin(0x77);
}