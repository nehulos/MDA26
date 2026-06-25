#ifndef LEDS_H
#define LEDS_H

#include <Arduino.h>

extern const int* ledPins;
extern size_t ledCount;

template <size_t N>
void initLeds(const int (&pins)[N])
{
    ledPins = pins;
    ledCount = N;

    for (size_t i = 0; i < N; i++)
    {
        pinMode(ledPins[i], OUTPUT);
        digitalWrite(ledPins[i], LOW);
    }
}

void setLed(size_t index, bool state);

void blinkLed(
    size_t index,
    unsigned long onTime,
    unsigned long offTime,
    int repetitions
);

#endif