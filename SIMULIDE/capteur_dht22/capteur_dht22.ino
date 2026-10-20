#include <DHT.h>

#define DHT_PIN A0
#define DHT_TYPE DHT22

#define LED_VERTE 3
#define LED_ROUGE 12

DHT dht(DHT_PIN, DHT_TYPE);

void setup()
{
    pinMode(LED_VERTE, OUTPUT);
    pinMode(LED_ROUGE, OUTPUT);

    digitalWrite(LED_VERTE, LOW);
    digitalWrite(LED_ROUGE, LOW);

    dht.begin();
}

void loop()
{
    // Attendre entre deux mesures
    delay(2000);

    float temperature = dht.readTemperature();

    // Si la lecture echoue, eteindre les deux LED
    if (isnan(temperature))
    {
        digitalWrite(LED_VERTE, LOW);
        digitalWrite(LED_ROUGE, LOW);
        return;
    }

    if (temperature < 20.0)
    {
        digitalWrite(LED_VERTE, HIGH);
        digitalWrite(LED_ROUGE, LOW);
    }
    else
    {
        digitalWrite(LED_VERTE, LOW);
        digitalWrite(LED_ROUGE, HIGH);
    }
}