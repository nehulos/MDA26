#include "leds.h"

const int* ledPins = nullptr;
size_t ledCount = 0;

void setLed(size_t index, bool state)
{
    if (index >= ledCount)
        return;

    digitalWrite(
        ledPins[index],
        state ? HIGH : LOW
    );
}

void blinkLed(
    size_t index,
    unsigned long onTime,
    unsigned long offTime,
    int repetitions
)
{
    if (index >= ledCount)
        return;

    int pin = ledPins[index];

    for (int i = 0; i < repetitions; i++)
    {
        digitalWrite(pin, HIGH);
        delay(onTime);

        digitalWrite(pin, LOW);
        delay(offTime);
    }
}