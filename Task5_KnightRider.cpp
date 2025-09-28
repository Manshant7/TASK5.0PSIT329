#include "Task5_KnightRider.h"

const uint8_t ledPins[LED_COUNT] = {3, 4, 5, 6, 7, 8};
const uint8_t buttonPin = 2;

// Variables
volatile uint8_t state = 0;
int ledIndex = 0;
int direction = 1;  
unsigned long lastUpdate = 0;
const unsigned long interval = 100; 

void buttonISR() {
  static unsigned long lastPress = 0;
  unsigned long now = millis();

  if (now - lastPress > 200) {
    state++;
    if (state > 3) state = 1;
    lastPress = now;
  }
}

void initLEDs() {
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
}

void initButton() {
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING);
}

void runKnightRider() {
  unsigned long now = millis();

  if (state == 1 && (now - lastUpdate >= interval)) {
    lastUpdate = now;

    for (uint8_t i = 0; i < LED_COUNT; i++) {
      digitalWrite(ledPins[i], LOW);
    }

    digitalWrite(ledPins[ledIndex], HIGH);

    ledIndex += direction;
    if (ledIndex == LED_COUNT - 1 || ledIndex == 0) {
      direction = -direction;  // bounce
    }
  }
  else if (state == 3) {
    
    for (uint8_t i = 0; i < LED_COUNT; i++) {
      digitalWrite(ledPins[i], LOW);
    }
    ledIndex = 0;
    direction = 1;
    state = 0;  
  }
}
