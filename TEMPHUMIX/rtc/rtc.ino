
#include <Wire.h>
#include <rgb_lcd.h>
#include <DHT.h>
#include <RTC.h>

// Configuration des composants
const byte LED_VIE = 2;
const byte DHT_PIN = A0;
const byte DHT_TYPE = DHT22;

rgb_lcd lcd;
DHT dht(DHT_PIN, DHT_TYPE);

// Machine d'etat : 4 etapes de 500 ms
byte etat = 0;

// Affichage de la date et de l'heure du RTC
void afficherDateHeure() {
  RTCTime maintenant;
  RTC.getTime(maintenant);

  char ligne[17];

  // Format compact pour le LCD 16 caracteres
  snprintf(ligne, sizeof(ligne),
           "%02d%02d%02d %02d:%02d:%02d",
           maintenant.getDayOfMonth(),
           Month2int(maintenant.getMonth()),
           maintenant.getYear() % 100,
           maintenant.getHour(),
           maintenant.getMinutes(),
           maintenant.getSeconds());

  lcd.setCursor(0, 0);
  lcd.print(ligne);
  lcd.print(" ");
}

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

  // Initialisation de la LED de vie
  pinMode(LED_VIE, OUTPUT);
  tone(LED_VIE, 2);

  // Initialisation du LCD et du DHT22
  lcd.begin(16, 2);
  lcd.setRGB(0, 0, 255);
  lcd.clear();
  dht.begin();

  // Initialisation de l'horloge RTC
  RTC.begin();

  RTCTime dateInitiale(21, Month::OCTOBER, 2026,
                       14, 13, 0,
                       DayOfWeek::WEDNESDAY,
                       SaveLight::SAVING_TIME_ACTIVE);

  RTC.setTime(dateInitiale);

  afficherDateHeure();

  lcd.setCursor(0, 1);
  lcd.print("Attente mesure");
}

void loop() {
  // Machine d'etat avec une periode de 500 ms
  switch (etat) {
    case 0:
      afficherDateHeure();
      lireEtAfficher();
      break;

    case 1:
      break;

    case 2:
      afficherDateHeure();
      break;

    case 3:
      break;
  }

  etat = (etat + 1) % 4;
  delay(500);
}
