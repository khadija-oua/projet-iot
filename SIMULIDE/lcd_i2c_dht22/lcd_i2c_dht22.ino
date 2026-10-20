#include <Wire.h>
#include <LiquidCrystal_AIP31068_I2C.h>
#include <DHT.h>

LiquidCrystal_AIP31068_I2C lcd(0x3E, 16, 2);
DHT dht(A0, DHT22);

void setup()
{
    Wire.begin();
    lcd.begin(16, 2);
    dht.begin();

    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("IMT NE - PPI");
}

void loop()
{
    delay(2000);

    float temperature = dht.readTemperature();
    float humidite = dht.readHumidity();

    // Effacer uniquement la deuxieme ligne
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);

    if (isnan(temperature) || isnan(humidite))
    {
        lcd.print("Erreur capteur");
        return;
    }

    lcd.print("T ");
    lcd.print(temperature, 1);
    lcd.print("C/H ");
    lcd.print(humidite, 1);
    lcd.print("%");
}