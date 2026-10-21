
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoMqttClient.h>
#include <RTC.h>
#include <Wire.h>
#include <rgb_lcd.h>
#include <DHT.h>
#include "arduino_secrets.h"

// Configuration des composants
const byte LED_VIE = 2;
const byte DHT_PIN = A0;
const char VERSION[] = "1.0";

rgb_lcd lcd;
DHT dht(DHT_PIN, DHT22);

// Configuration WiFi et NTP
WiFiUDP udpNTP;
IPAddress serveurNTP(192, 168, 1, 254);
NTPClient clientNTP(udpNTP, serveurNTP, 7200, 60000);

// Configuration MQTT
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

IPAddress broker(192, 168, 1, 1);
const int PORT_MQTT = 1883;
const char CLIENT_ID[] = "Id0_Client_48763";

const char TOPIC_VERSION[] = "/iot/VERSION_48763";
const char TOPIC_TEMP[] = "/iot/TEMPERATURE_48763";
const char TOPIC_HUM[] = "/iot/HUMIDITY_48763";
const char TOPIC_DATE[] = "/iot/GET_DATE_48763";

bool rtcSynchronise = false;
byte etat = 0;

unsigned long dernierEssaiNTP = 0;
unsigned long dernierEssaiMQTT = 0;

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

// Synchronisation de l'horloge RTC avec NTP
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

// Initialisation de la connexion MQTT
int setup_mqtt() {
  mqttClient.setId(CLIENT_ID);
  mqttClient.setUsernamePassword(
    SECRET_MQTT_USER,
    SECRET_MQTT_PASS
  );

  if (!mqttClient.connect(broker, PORT_MQTT)) {
    int erreur = mqttClient.connectError();
    Serial.print("Erreur MQTT : ");
    Serial.println(erreur);
    return erreur;
  }

  Serial.println("MQTT connecte au broker");
  return 0;
}

// Publication d'un message sur un topic
void publier(const char* topic, const String& valeur) {
  if (!mqttClient.connected()) {
    return;
  }

  mqttClient.beginMessage(topic);
  mqttClient.print(valeur);
  mqttClient.endMessage();

  Serial.print(topic);
  Serial.print(" : ");
  Serial.println(valeur);
}

// Lecture de la date et de l'heure
String obtenirDateHeure() {
  RTCTime maintenant;
  RTC.getTime(maintenant);

  char dateHeure[20];

  snprintf(dateHeure, sizeof(dateHeure),
           "%02d/%02d/%04d %02d:%02d:%02d",
           maintenant.getDayOfMonth(),
           Month2int(maintenant.getMonth()),
           maintenant.getYear(),
           maintenant.getHour(),
           maintenant.getMinutes(),
           maintenant.getSeconds());

  return String(dateHeure);
}

// Affichage de la date et de l'heure sur le LCD
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

// Acquisition et publication des 4 variables
void lireEtPublier() {
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

  if (mqttClient.connected()) {
    publier(TOPIC_VERSION, String(VERSION));
    publier(TOPIC_TEMP, String(temperature, 1));
    publier(TOPIC_HUM, String(humidite, 1));

    if (rtcSynchronise) {
      publier(TOPIC_DATE, obtenirDateHeure());
    }
  }
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

  clientNTP.begin();

  if (connecte) {
    rtcSynchronise = synchroniserRTC();
    dernierEssaiNTP = millis();

    // Connexion initiale au broker
    setup_mqtt();
    dernierEssaiMQTT = millis();
  }

  lcd.clear();
}

void loop() {
  // Maintien de la connexion MQTT
  if (mqttClient.connected()) {
    mqttClient.poll();
  } else if (WiFi.status() == WL_CONNECTED &&
             millis() - dernierEssaiMQTT >= 5000) {
    dernierEssaiMQTT = millis();
    setup_mqtt();
  }

  // Nouvelle tentative NTP si necessaire
  if (!rtcSynchronise &&
      WiFi.status() == WL_CONNECTED &&
      millis() - dernierEssaiNTP >= 30000) {
    dernierEssaiNTP = millis();
    rtcSynchronise = synchroniserRTC();
  }

  // Machine d'etat : 4 etapes de 500 ms
  switch (etat) {
    case 0:
      afficherDateHeure();
      lireEtPublier();
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
