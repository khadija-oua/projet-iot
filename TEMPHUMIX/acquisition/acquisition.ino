
#include <Wire.h>
#include <rgb_lcd.h>
#include <DHT.h>

// Configuration des composants
const byte LED_VIE = 2;
const byte DHT_PIN = A0;
const byte DHT_TYPE = DHT22;

rgb_lcd lcd;
DHT dht(DHT_PIN, DHT_TYPE);

// Etat de la machine pour une acquisition toutes les 2 secondes
byte etat = 0;

// Lecture du capteur et affichage des mesures
void lireEtAfficher() {
  float temperature = dht.readTemperature();
  float humidite = dht.readHumidity();

  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);

  if (isnan(temperature) || isnan(humidite)) {
    lcd.print("Erreur DHT22");
    Serial.println("Erreur lecture DHT22");
    return;
  }

  lcd.print("T ");
  lcd.print(temperature, 1);
  lcd.print("C/H ");
  lcd.print(humidite, 1);
  lcd.print("%");

  Serial.print("Temperature : ");
  Serial.print(temperature, 1);
  Serial.println(" C");

  Serial.print("Humidite : ");
  Serial.print(humidite, 1);
  Serial.println(" %");
}

void setup() {
  Serial.begin(9600);

  // Initialisation de la LED de vie a 2 Hz
  pinMode(LED_VIE, OUTPUT);
  tone(LED_VIE, 2);

  // Initialisation du LCD
  lcd.begin(16, 2);
  lcd.setRGB(0, 0, 255);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("IMT NE - PPI");

  lcd.setCursor(0, 1);
  lcd.print("Attente mesure");

  // Initialisation du capteur
  dht.begin();
}

void loop() {
  // Machine d'etat : 4 etapes de 500 ms
  switch (etat) {
    case 0:
      lireEtAfficher();
      break;

    case 1:
    case 2:
    case 3:
      // Attente de la prochaine acquisition
      break;
  }

  etat = (etat + 1) % 4;
  delay(500);
}
