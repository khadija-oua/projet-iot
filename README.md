# Projet IoT — TempHumiX

Simulation d’un capteur de température et d’humidité avec Arduino UNO R3, un DHT22 et un écran LCD I2C.

## Fonctionnalités

- Clignotement d’une LED.
- Chenillard de 10 LED.
- Commande de LED selon la température.
- Affichage de la température et de l’humidité sur LCD.
- Horloge logicielle et transmission des mesures toutes les 2 secondes.

## Organisation du dépôt

| Dossier dans `SIMULIDE/` | Description |
|---|---|
| `clignotement_led/` | LED clignotante |
| `bargraphe/` | Chenillard de 10 LED à 100 ms |
| `capteur_dht22/` | Capteur DHT22 et indicateurs lumineux |
| `lcd_i2c/` | Affichage HELLO WORLD |
| `lcd_i2c_dht22/` | Affichage des mesures |
| `TempHumiX/` | Montage intégré avec horloge et terminal série |

Chaque dossier contient le programme Arduino `.ino` et le circuit SimulIDE `.sim1`.

## Outils utilisés

- SimulIDE
- Arduino IDE
- Git et GitHub

Bibliothèques : **DHT sensor library** d’Adafruit et **LiquidCrystal AIP31068**.

## Lancer une simulation

1. Ouvrir le fichier `.ino` dans Arduino IDE.
2. Sélectionner la carte **Arduino Uno**.
3. Installer les bibliothèques nécessaires.
4. Cliquer sur **Sketch → Export Compiled Binary**.
5. Ouvrir le circuit `.sim1` dans SimulIDE.
6. Faire un clic droit sur l’Arduino → **Load firmware**.
7. Charger le fichier `.ino.hex` sans `with_bootloader`.
8. Lancer la simulation.

Pour TempHumiX, ouvrir le **Serial Terminal** réglé à **9600 bauds** pour consulter la date, l’heure et les mesures.