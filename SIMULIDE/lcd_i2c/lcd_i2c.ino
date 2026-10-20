#include <Wire.h>
#include <LiquidCrystal_AIP31068_I2C.h>

// Adresse I2C, nombre de colonnes, nombre de lignes
LiquidCrystal_AIP31068_I2C lcd(0x3E, 16, 2);

void setup()
{
    Wire.begin();
    lcd.begin(16, 2);
    lcd.clear();

    // Premiere colonne, premiere ligne
    lcd.setCursor(0, 0);
    lcd.print("HELLO");

    // Colonne 8, deuxieme ligne
    lcd.setCursor(7, 1);
    lcd.print("WORLD");
}

void loop()
{
    // Le texte reste affiche : rien a repeter
}