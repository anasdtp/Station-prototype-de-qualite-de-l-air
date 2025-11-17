# Projet Prévaux - Station de Mesure Environnementale

## 📋 Description

Station de mesure environnementale développée avec Arduino qui combine la mesure de paramètres météorologiques (température, humidité, pression) et de qualité de l'air (particules fines) avec un affichage visuel sur anneau NeoPixel.

Le système calcule une corrélation entre la température et la qualité de l'air pour fournir un indice environnemental global affiché visuellement.

## 🎯 Fonctionnalités

- **Mesures Météorologiques** : Température, humidité, pression atmosphérique et altitude approximative
- **Qualité de l'Air** : Mesure des particules fines PM1.0, PM2.5 et PM10 avec calcul d'indices AQI
- **Affichage Visuel** : Anneau NeoPixel de 12 LEDs avec codage couleur et animation
- **Corrélation Environnementale** : Algorithme de calcul de l'impact croisé température/qualité de l'air
- **Diagnostic Complet** : Messages détaillés de diagnostic et de dépannage

## 🔧 Matériel Requis

### Microcontrôleur
- **Arduino Nano Every** (ou compatible ATmega4809)

### Capteurs
- **BME280** : Capteur température/humidité/pression (I2C)
- **PMS5003** : Capteur de particules fines (UART)

### Affichage
- **NeoPixel Ring** : Anneau de 12 LEDs WS2812B

### Connexions

#### BME280 (I2C)
```
BME280    Arduino Nano Every
------    ------------------
VCC       3.3V
GND       GND
SDA       A4 (SDA)
SCL       A5 (SCL)
```

#### PMS5003 (UART via SoftwareSerial)
```
PMS5003   Arduino Nano Every
-------   ------------------
VCC       5V
GND       GND
TX        Pin 4 (RX Software)
RX        Pin 5 (TX Software)
```

#### NeoPixel Ring
```
NeoPixel  Arduino Nano Every
--------  ------------------
VCC       5V
GND       GND
DIN       Pin 6
```

## 📦 Installation

### Prérequis
- [PlatformIO](https://platformio.org/) installé
- VS Code avec l'extension PlatformIO

### Configuration du Projet

1. **Cloner le projet** :
```bash
git clone https://github.com/anasdtp/Station-prototype-de-qualite-de-l-air
cd Station-prototype-de-qualite-de-l-air
```

2. **Installer les dépendances** :
Les bibliothèques sont automatiquement installées grace à PlatformIO :
```ini
lib_deps = 
    adafruit/Adafruit Unified Sensor@^1.1.15
    adafruit/Adafruit BME280 Library@^2.3.0
    adafruit/Adafruit NeoPixel@^1.15.2
    adafruit/Adafruit PM25 AQI Sensor@^2.0.0
```

3. **Compiler et téléverser** :
```bash
pio run --target upload
```
ou bien cliquer sur l'icone "Upload"

4. **Ouvrir le moniteur série** :
```bash
pio device monitor --baud 115200
```
ou bien cliquer sur l'icone "Serial Monitor"

## 💡 Utilisation

### Démarrage
1. Connecter l'Arduino via USB
2. Ouvrir le moniteur série (115200 bauds)
3. Observer la séquence d'initialisation des capteurs
4. Le système affiche un motif rouge en cas d'erreur, puis passe en mode mesure

### Interprétation de l'Affichage LED

#### Nombre de LEDs (Température)
- **0-12 LEDs** : Proportionnel à la température (0-35°C)
- Plus il fait chaud, plus de LEDs sont allumées

#### Couleur des LEDs (Qualité de l'Air)
| Couleur | AQI | Qualité |
|---------|-----|---------|
| 🟢 Vert | 0-12 | Excellent |
| 🟡 Jaune-Vert | 13-35 | Bon |
| 🟡 Jaune | 36-55 | Modéré |
| 🟠 Orange | 56-150 | Mauvais pour sensibles |
| 🔴 Rouge | 151-250 | Mauvais |
| 🟣 Violet | >250 | Très mauvais |

### Effets Spéciaux
- **Animation de remplissage** : Les LEDs s'allument progressivement
- **Pulsation** : Indique la corrélation environnementale
- **Motif d'erreur** : Rouge fixe en cas de problème capteur

## 🧮 Algorithme de Corrélation

Le système calcule un indice de corrélation température/qualité de l'air :

```cpp
// Base : 100 - (AQI / 200) * 100
correlation = 100 - (max(pm25_aqi, pm10_aqi) / 200) * 100

// Corrections météorologiques :
if (température ≤ 5°C)  → -10% (inversion thermique)
if (température ≥ 27°C) → -7%  (pollution photochimique)
```

## 📊 Sortie Série

Le système affiche toutes les 2 secondes :

```
=== DONNÉES ENVIRONNEMENTALES ===
Temp: 22.5 °C | Humidité: 45.2 % | Pression: 1013.2 hPa | Alt: 145.3 m

=== QUALITÉ DE L'AIR PMS5003 ===
Concentrations Standard (μg/m³)
PM 1.0: 8    PM 2.5: 12    PM 10: 15

=== AFFICHAGE LED ENVIRONNEMENTAL ===
Température: 22.5 °C -> 8 LEDs allumées
PM2.5: 12.0 μg/m³ -> Couleur: VERT (Excellent)
Corrélation Température-Qualité de l'air: 94%
```

## 🐛 Dépannage

### Erreurs Communes

#### BME280 non détecté
- Vérifier le câblage I2C (SDA/SCL)
- Vérifier l'alimentation (3.3V)
- Tester les adresses I2C 0x76 et 0x77

#### PMS5003 ne répond pas
- Vérifier l'alimentation (5V)
- Contrôler le câblage UART (TX/RX croisés)
- Attendre 30 secondes après mise sous tension
- Vérifier la configuration des pins (4 et 5)

#### LEDs ne s'allument pas
- Vérifier l'alimentation NeoPixel (5V)
- Contrôler la connexion DIN (Pin 6)
- Tester avec un motif simple

### Messages de Diagnostic

Le code inclut des messages détaillés pour identifier rapidement les problèmes :
- Diagnostic de connexion série
- Test d'initialisation des capteurs
- Vérification de la disponibilité des données

## ⚙️ Configuration

### Constantes modifiables dans `main.cpp` :

```cpp
#define PIN_NEOPIX 6         // Pin du NeoPixel Ring
#define NPIX 12              // Nombre de LEDs
#define SEALEVEL_HPA 1013.25 // Pression niveau mer (à ajuster)
#define PMS_RX_PIN 4         // Pin RX pour PMS5003
#define PMS_TX_PIN 5         // Pin TX pour PMS5003
```

### Paramètres de mesure :
- **Fréquence** : Mesure toutes les 2 secondes
- **Luminosité LEDs** : 50/255 (réglable)
- **Vitesse série** : 115200 bauds

## 🔄 Évolutions Possibles

- [ ] Interface web pour visualisation à distance
- [ ] Stockage des données sur carte SD
- [ ] Alertes par notification
- [ ] Calibration automatique des capteurs
- [ ] Support de capteurs supplémentaires (CO2, COV)
- [ ] Mode économie d'énergie avec réveil temporisé

## 📄 Licence

Ce projet est sous licence libre. Vous pouvez l'utiliser, le modifier et le distribuer selon vos besoins.

## 👥 Contribution

Les contributions sont les bienvenues ! N'hésitez pas à :
- Signaler des bugs
- Proposer des améliorations
- Ajouter de nouvelles fonctionnalités

## 📞 Support

Pour toute question ou problème :
1. Consulter la section dépannage
2. Vérifier les messages du moniteur série
3. Contrôler le câblage selon le schéma fourni