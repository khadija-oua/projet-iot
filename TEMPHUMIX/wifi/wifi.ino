
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <RTC.h>
#include <Wire.h>
#include <rgb_lcd.h>
#include <DHT.h>
#include "arduino_secrets.h"

// Configuration des composants
const byte LED_VIE = 2;
const byte DHT_PIN = A0;
const char VERSION[] = "V1.0";

rgb_lcd lcd;
DHT dht(DHT_PIN, DHT22);

// Configuration du serveur NTP
WiFiUDP udpNTP;
IPAddress serveurNTP(192, 168, 1, 254);
NTPClient clientNTP(udpNTP, serveurNTP, 7200, 60000);

bool rtcSynchronise = false;
bool ntpTente = false;
byte etat = 0;
unsigned long dernierEssaiNTP = 0;

// Connexion au reseau WiFi
bool setup_wifi() {
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Module WiFi indisponible");
    return false;
  }

  WiFi.begin(SECRET_SSID, SECRET_PASS);
  unsigned long debut = millis();

  while (millis() - debut < 15000) {
    if (WiFi.status() == WL_CONNECTED &&
        WiFi.localIP()[0] != 0) {
      Serial.println("WiFi connecte");
      Serial.print("Adresse IP : ");
      Serial.println(WiFi.localIP());
      return true;
    }
    delay(500);
  }

  Serial.println("Connexion WiFi echouee");
  return false;
}

// Synchronisation du RTC avec le serveur NTP
bool synchroniserRTC() {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  if (!clientNTP.forceUpdate()) {
    Serial.println("Synchronisation NTP echouee");
    return false;
  }

  RTCTime dateNTP(clientNTP.getEpochTime());
  RTC.setTime(dateNTP);

  Serial.println("RTC synchronise avec NTP");
  return true;
}

// Affichage de la date et de l'heure
void afficherDateHeure() {
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);

  if (!rtcSynchronise) {
    lcd.print("NTP non synchro");
    return;
  }

  RTCTime maintenant;
  RTC.getTime(maintenant);

  char ligne[17];
  snprintf(ligne, sizeof(ligne),
           "%02d%02d%02d %02d:%02d:%02d",
           maintenant.getDayOfMonth(),
           Month2int(maintenant.getMonth()),
           maintenant.getYear() % 100,
           maintenant.getHour(),
           maintenant.getMinutes(),
           maintenant.getSeconds());

  lcd.print(ligne);
}

// Lecture et affichage des mesures
void lireEtAfficher() {
  float temperature = dht.readTemperature();
  float humidite = dht.readHumidity();

  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);

  if (isnan(temperature) || isnan(humidite)) {
    lcd.print("Erreur DHT22");
    return;
  }

  lcd.print("T ");
  lcd.print(temperature, 1);
  lcd.print("C/H ");
  lcd.print(humidite, 1);
  lcd.print("%");
}

void setup() {
  Serial.begin(9600);

  // Initialisation des composants
  pinMode(LED_VIE, OUTPUT);
  tone(LED_VIE, 2);

  lcd.begin(16, 2);
  lcd.setRGB(0, 0, 255);
  lcd.clear();

  dht.begin();
  RTC.begin();

  // Connexion WiFi
  lcd.print("Connexion WiFi");
  bool connecte = setup_wifi();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(VERSION);

  lcd.setCursor(0, 1);
  if (connecte) {
    lcd.print(WiFi.localIP());
  } else {
    lcd.print("WiFi echec");
  }

  // Initialisation du client NTP
  clientNTP.begin();
}

void loop() {
  // Synchronisation NTP avec nouvelle tentative si echec
  if (!rtcSynchronise &&
      WiFi.status() == WL_CONNECTED &&
      (!ntpTente || millis() - dernierEssaiNTP >= 30000)) {

    ntpTente = true;
    dernierEssaiNTP = millis();
    rtcSynchronise = synchroniserRTC();
  }

  // Machine d'etat de 500 ms
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
