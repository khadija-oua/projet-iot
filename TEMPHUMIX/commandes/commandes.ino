#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoMqttClient.h>
#include <RTC.h>
#include <Wire.h>
#include <rgb_lcd.h>
#include <DHT.h>
#include <FspTimer.h>
#include "arduino_secrets.h"

// Configuration des composants
const byte LED_VIE = 2;
const byte DHT_PIN = A0;
const byte PREMIERE_LED_BARGRAPH = 3;  // D3 a D12 : cablage a confirmer
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
const char TOPIC_COMMAND[] = "/iot/COMMAND_48763";
const char TOPIC_BARGRAPH[] = "/iot/BARGRAPH_48763";
const char TOPIC_BLINK[] = "/iot/BLINK_48763";

bool rtcSynchronise = false;
bool mesuresActives = true;
bool demandeNTP = false;
byte etat = 0;
unsigned long dernierEssaiNTP = 0;
unsigned long dernierEssaiMQTT = 0;
unsigned int periodePersonnalisee = 600;

// Timer materiel : pas de 50 ms, independant de delay(500)
FspTimer timerLed;
volatile unsigned int demiPeriodeTicks = 5;  // 250 ms : LED a 2 Hz
volatile unsigned int compteurTicks = 0;
volatile bool etatLed = false;

void interruptionLed(timer_callback_args_t *args) {
  compteurTicks++;
  if (compteurTicks >= demiPeriodeTicks) {
    compteurTicks = 0;
    etatLed = !etatLed;
    digitalWrite(LED_VIE, etatLed ? HIGH : LOW);
  }
}

// Reglage de la periode complete du clignotement
void definirPeriodeLed(unsigned int periodeMs) {
  noInterrupts();
  demiPeriodeTicks = periodeMs / 100; // Demi-periode / 50 ms
  compteurTicks = 0;
  etatLed = false;
  digitalWrite(LED_VIE, LOW);
  interrupts();
}

bool initialiserLed() {
  pinMode(LED_VIE, OUTPUT);
  digitalWrite(LED_VIE, LOW);

  uint8_t typeTimer = GPT_TIMER;
  int8_t canal = FspTimer::get_available_timer(typeTimer);
  if (canal < 0) return false;
  if (!timerLed.begin(TIMER_MODE_PERIODIC, typeTimer, canal,
                      20.0f, 0.0f, interruptionLed)) return false;
  if (!timerLed.setup_overflow_irq()) return false;
  if (!timerLed.open()) return false;
  return timerLed.start();
}

// Affichage binaire d'une valeur de 0 a 1023 sur 10 LED
void afficherBargraph(unsigned int valeur) {
  for (byte i = 0; i < 10; i++) {
    digitalWrite(PREMIERE_LED_BARGRAPH + i,
                 (valeur & (1U << i)) ? HIGH : LOW);
  }
}

// Conversion et verification d'une valeur entiere recue
bool lireEntier(String message, long &valeur) {
  message.trim();
  if (message.length() == 0) return false;
  for (unsigned int i = 0; i < message.length(); i++) {
    if (message[i] < '0' || message[i] > '9') return false;
  }
  valeur = message.toInt();
  return true;
}

// Connexion au reseau WiFi
bool setup_wifi() {
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Module WiFi indisponible");
    return false;
  }

  WiFi.begin(SECRET_SSID, SECRET_PASS);
  unsigned long debut = millis();

  while (millis() - debut < 15000) {
    if (WiFi.status() == WL_CONNECTED && WiFi.localIP()[0] != 0) {
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
  if (WiFi.status() != WL_CONNECTED) return false;
  if (!clientNTP.forceUpdate()) {
    Serial.println("Synchronisation NTP echouee");
    return false;
  }

  RTCTime dateNTP(clientNTP.getEpochTime());
  RTC.setTime(dateNTP);
  Serial.println("RTC synchronise avec NTP");
  return true;
}

// Reception des commandes MQTT
void onMqttMessage(int tailleMessage) {
  String topic = mqttClient.messageTopic();
  String message = "";
  while (mqttClient.available()) {
    message += (char)mqttClient.read();
  }

  Serial.print("Topic recu : ");
  Serial.println(topic);
  Serial.print("Message : ");
  Serial.println(message);

  long valeur;
  if (!lireEntier(message, valeur)) {
    Serial.println("Message invalide : entier attendu");
    return;
  }

  if (topic == TOPIC_COMMAND) {
    switch (valeur) {
      case 10:
        // Demander une nouvelle synchronisation NTP
        demandeNTP = true;
        Serial.println("Synchronisation NTP demandee");
        break;

      case 20:
        mesuresActives = true;
        Serial.println("Mesures demarrees");
        break;

      case 30:
        mesuresActives = false;
        Serial.println("Mesures arretees");
        break;

      case 40:
        definirPeriodeLed(1000);
        Serial.println("Clignotement lent : 1000 ms");
        break;

      case 50:
        definirPeriodeLed(200);
        Serial.println("Clignotement rapide : 200 ms");
        break;

      case 60:
        definirPeriodeLed(periodePersonnalisee);
        Serial.print("Clignotement personnalise : ");
        Serial.print(periodePersonnalisee);
        Serial.println(" ms");
        break;

      default:
        Serial.println("Code commande inconnu");
        break;
    }
  }
  else if (topic == TOPIC_BARGRAPH) {
    if (valeur >= 0 && valeur <= 1023) {
      afficherBargraph((unsigned int)valeur);
      Serial.println("Bargraphe mis a jour");
    } else {
      Serial.println("Bargraphe : valeur attendue de 0 a 1023");
    }
  }
  else if (topic == TOPIC_BLINK) {
    // La periode doit etre positive et multiple de 100 ms
    if (valeur >= 100 && valeur <= 60000 && valeur % 100 == 0) {
      periodePersonnalisee = (unsigned int)valeur;
      Serial.println("Periode personnalisee memorisee");
    } else {
      Serial.println("BLINK : multiple de 100 ms attendu");
    }
  }

  Serial.println("----------------");
}

// Connexion MQTT et abonnement aux trois topics
int setup_mqtt() {
  mqttClient.setId(CLIENT_ID);
  mqttClient.setUsernamePassword(SECRET_MQTT_USER, SECRET_MQTT_PASS);

  if (!mqttClient.connect(broker, PORT_MQTT)) {
    int erreur = mqttClient.connectError();
    Serial.print("Erreur MQTT : ");
    Serial.println(erreur);
    return erreur;
  }

  Serial.println("MQTT connecte au broker");
  mqttClient.onMessage(onMqttMessage);

  bool ok1 = mqttClient.subscribe(TOPIC_COMMAND);
  bool ok2 = mqttClient.subscribe(TOPIC_BARGRAPH);
  bool ok3 = mqttClient.subscribe(TOPIC_BLINK);

  if (ok1 && ok2 && ok3) {
    Serial.println("Souscriptions MQTT reussies");
  } else {
    Serial.println("Erreur de souscription MQTT");
  }
  return 0;
}

// Publication MQTT sans encombrer le moniteur serie
void publier(const char* topic, const String& valeur) {
  if (!mqttClient.connected()) return;
  mqttClient.beginMessage(topic);
  mqttClient.print(valeur);
  mqttClient.endMessage();
}

// Lecture de la date et de l'heure du RTC
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

// Acquisition et publication des quatre variables
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
    if (rtcSynchronise) publier(TOPIC_DATE, obtenirDateHeure());
  }
}

void setup() {
  Serial.begin(9600);

  // Initialisation des composants
  if (!initialiserLed()) {
    Serial.println("Erreur timer LED");
  }

  for (byte i = 0; i < 10; i++) {
    pinMode(PREMIERE_LED_BARGRAPH + i, OUTPUT);
  }
  afficherBargraph(0);

  lcd.begin(16, 2);
  lcd.setRGB(0, 0, 255);
  lcd.clear();
  dht.begin();
  RTC.begin();

  lcd.print("Connexion WiFi");
  bool connecte = setup_wifi();
  clientNTP.begin();

  if (connecte) {
    rtcSynchronise = synchroniserRTC();
    dernierEssaiNTP = millis();
    setup_mqtt();
    dernierEssaiMQTT = millis();
  }
  lcd.clear();
}

void loop() {
  // Maintien de la liaison MQTT
  if (mqttClient.connected()) {
    mqttClient.poll();
  } else if (WiFi.status() == WL_CONNECTED &&
             millis() - dernierEssaiMQTT >= 5000) {
    dernierEssaiMQTT = millis();
    setup_mqtt();
  }

  // Synchronisation sur commande 10 ou nouvel essai automatique
  if (demandeNTP) {
    demandeNTP = false;
    if (synchroniserRTC()) {
      rtcSynchronise = true;
      Serial.println("RTC synchronise");
    } else {
      Serial.println("Echec synchronisation RTC");
    }
    dernierEssaiNTP = millis();
  }
  else if (!rtcSynchronise &&
           WiFi.status() == WL_CONNECTED &&
           millis() - dernierEssaiNTP >= 30000) {
    dernierEssaiNTP = millis();
    rtcSynchronise = synchroniserRTC();
  }

  // Machine d'etat : quatre etapes de 500 ms
  switch (etat) {
    case 0:
      afficherDateHeure();
      if (mesuresActives) lireEtPublier();
      break;
    case 2:
      afficherDateHeure();
      break;
  }

  etat = (etat + 1) % 4;
  delay(500);
}
