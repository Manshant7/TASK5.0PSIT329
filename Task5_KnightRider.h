#ifndef TASK5_KNIGHTRIDER_H
#define TASK5_KNIGHTRIDER_H

#include <Arduino.h>

#define LED_COUNT 6
extern const uint8_t ledPins[LED_COUNT];
extern const uint8_t buttonPin;

void initLEDs();
void initButton();
void runKnightRider();

#endif
