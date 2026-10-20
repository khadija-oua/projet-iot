// Dix LED branchees sur les sorties D3 a D12
const int PREMIERE_LED = 3;
const int DERNIERE_LED = 12;
const int DUREE = 100; // millisecondes

void setup()
{
    for (int broche = PREMIERE_LED; broche <= DERNIERE_LED; broche++)
    {
        pinMode(broche, OUTPUT);
        digitalWrite(broche, LOW);
    }
}

void loop()
{
    for (int broche = PREMIERE_LED; broche <= DERNIERE_LED; broche++)
    {
        digitalWrite(broche, HIGH);
        delay(DUREE);
        digitalWrite(broche, LOW);
    }
}