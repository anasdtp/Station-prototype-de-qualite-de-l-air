#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_NeoPixel.h>

#define PIN_NEOPIX 6     // Broche reliée à DIN du NeoPixel Ring
#define NPIX 12          // Nombre de LEDs sur l'anneau
#define SEALEVEL_HPA 1013.25  // Pression atmosphérique au niveau de la mer (à ajuster selon ta localisation)

Adafruit_BME280 bme;     // Objet BME280 (I2C)
Adafruit_NeoPixel strip(NPIX, PIN_NEOPIX, NEO_GRB + NEO_KHZ800);

// Convertit la température (°C) en couleur (bleu à rouge)
uint32_t tempToColor(float tC) {
  float t = constrain(tC, 0.0, 35.0);  // Limite entre 0 et 35°C
  float ratio = t / 35.0;
  uint8_t r = (uint8_t)(255 * ratio);             // 0 → 255
  uint8_t g = (uint8_t)(255 * (1 - abs(ratio * 2 - 1))); // Vert au milieu
  uint8_t b = (uint8_t)(255 * (1 - ratio));       // 255 → 0
  return strip.Color(r, g, b);
}

// Allume un nombre de LED en fonction de l'humidité
void showHumidityBar(float hum, uint32_t color) {
  int n = map((int)constrain(hum, 0, 100), 0, 100, 0, NPIX); // 0 à 12 LEDs
  for (int i = 0; i < NPIX; i++) {
    strip.setPixelColor(i, (i < n) ? color : 0);
  }
  strip.show();
}

bool beginBME() {
  // Essaie adresse 0x76 puis 0x77
  return bme.begin(0x76) || bme.begin(0x77);
}

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(50);  // Réduit la luminosité du ring
  strip.show();             // Éteint toutes les LEDs

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
  float t = bme.readTemperature();        // Température (°C)
  float h = bme.readHumidity();           // Humidité (%)
  float p = bme.readPressure() / 100.0F;  // Pression (hPa)
  float alt = bme.readAltitude(SEALEVEL_HPA); // Altitude approx. (m)

  // Affichage via moniteur série
  Serial.print("Temp: "); Serial.print(t, 1); Serial.print(" °C | ");
  Serial.print("Humidité: "); Serial.print(h, 1); Serial.print(" % | ");
  Serial.print("Pression: "); Serial.print(p, 1); Serial.print(" hPa | ");
  Serial.print("Alt: "); Serial.print(alt, 1); Serial.println(" m");

  // Animation sur NeoPixel
  uint32_t col = tempToColor(t);
  showHumidityBar(h, col);

  delay(1000); // Pause 1 seconde entre chaque mesure
}
