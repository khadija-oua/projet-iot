#include <Wire.h>
#include <LiquidCrystal_AIP31068_I2C.h>
#include <DHT.h>
#include <SoftwareSerial.h>

const byte LED_VIE = 2;

// SoftwareSerial demande les broches dans l'ordre RX, TX
SoftwareSerial liaison(A2, A1);

LiquidCrystal_AIP31068_I2C lcd(0x3E, 16, 2);
DHT dht(A0, DHT22);

// Date et heure de depart, modifiables
int jour = 20;
int mois = 10;
int annee = 2026;
int heure = 17;
int minute = 12;
int seconde = 0;

unsigned long dernierClignotement = 0;
unsigned long derniereSeconde = 0;
unsigned long derniereMesure = 0;

bool etatLed = false;

void avancerHorloge()
{
    seconde++;

    if (seconde >= 60) {
        seconde = 0;
        minute++;
    }
    if (minute >= 60) {
        minute = 0;
        heure++;
    }
    if (heure >= 24) {
        heure = 0;
        jour++;
    }

    // Selon l'enonce : tous les mois ont 30 jours
    if (jour > 30) {
        jour = 1;
        mois++;
    }
    if (mois > 12) {
        mois = 1;
        annee++;
    }
}

void afficherHorloge()
{
    char ligne[17];

    // Format compact JJMM AA sans espaces : JJMMAA HH:MM:SS
    snprintf(ligne, sizeof(ligne),
             "%02d%02d%02d %02d:%02d:%02d",
             jour, mois, annee % 100,
             heure, minute, seconde);

    lcd.setCursor(0, 0);
    lcd.print(ligne);
    lcd.print(" ");
}

void lireEtEnvoyer()
{
    float temperature = dht.readTemperature();
    float humidite = dht.readHumidity();

    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);

    if (isnan(temperature) || isnan(humidite)) {
        lcd.print("Erreur capteur");
        liaison.println("Erreur lecture DHT22");
        return;
    }

    lcd.print("T ");
    lcd.print(temperature, 1);
    lcd.print("C/H ");
    lcd.print(humidite, 1);
    lcd.print("%");

    char dateHeure[24];

    snprintf(dateHeure, sizeof(dateHeure),
             "%02d/%02d/%04d %02d:%02d:%02d",
             jour, mois, annee, heure, minute, seconde);

    liaison.println(dateHeure);
    liaison.print("Temperature : ");
    liaison.print(temperature, 1);
    liaison.println(" C");

    liaison.print("Humidite : ");
    liaison.print(humidite, 1);
    liaison.println(" %");
    liaison.println();
}

void setup()
{
    pinMode(LED_VIE, OUTPUT);
    digitalWrite(LED_VIE, LOW);

    // Initialiser les sorties du bargraphe
    for (byte broche = 3; broche <= 12; broche++) {
        pinMode(broche, OUTPUT);
        digitalWrite(broche, LOW);
    }

    liaison.begin(9600);

    Wire.begin();
    lcd.begin(16, 2);
    lcd.clear();

    dht.begin();

    afficherHorloge();
    lcd.setCursor(0, 1);
    lcd.print("Attente mesure");

    unsigned long maintenant = millis();
    dernierClignotement = maintenant;
    derniereSeconde = maintenant;
    derniereMesure = maintenant;
}

void loop()
{
    unsigned long maintenant = millis();

    // 2 Hz = un cycle de 500 ms :
    // 250 ms allumee, puis 250 ms eteinte
    if (maintenant - dernierClignotement >= 250) {
        dernierClignotement += 250;
        etatLed = !etatLed;
        digitalWrite(LED_VIE, etatLed);
    }

    // Faire avancer l'horloge sans perdre les secondes ecoulees
    bool horlogeModifiee = false;

    while (maintenant - derniereSeconde >= 1000) {
        derniereSeconde += 1000;
        avancerHorloge();
        horlogeModifiee = true;
    }

    if (horlogeModifiee) {
        afficherHorloge();
    }

    // Acquisition et transmission toutes les deux secondes
    if (maintenant - derniereMesure >= 2000) {
        derniereMesure = maintenant;
        lireEtEnvoyer();
    }
}