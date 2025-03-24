#include <Ticker.h>

const byte INTERRUPT_PIN = 2; // Use a GPIO pin that supports interrupts
const byte TRIAC_PIN = 15; // Use a GPIO pin for TRIAC control
const byte TRIAC_PULSE_MICROS = 30;

const int FADE_MAX = 7000;
const int FADE_MIN = 10;
const int looptime = 5;
int fadeAmount = 10;

volatile bool triacOn;
volatile int period = FADE_MIN; // microseconds cut out from AC pulse

Ticker triacTicker;

void zeroCrossing() {
  triacOn = false; // triac turns off self at zero crossing
  triacTicker.detach(); // Stop the ticker
  triacTicker.once_us(period, triacPulse); // Set the next pulse once
}

void triacPulse() {
  if (triacOn) { // stop pulse
    digitalWrite(TRIAC_PIN, LOW);
    triacTicker.detach(); // Stop the ticker
  } else { // start pulse
    digitalWrite(TRIAC_PIN, HIGH);
    triacOn = true;
    triacTicker.once_us(TRIAC_PULSE_MICROS, triacPulse); // Set the pulse duration once
  }
}

void setup() {
  pinMode(TRIAC_PIN, OUTPUT);
  pinMode(INTERRUPT_PIN, INPUT_PULLUP); // Set the interrupt pin
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), zeroCrossing, RISING);
}

void loop() {
  // Update the period gradually
  period += fadeAmount;

  // Clamping to avoid edge cases
  if (period >= FADE_MAX) {
    period = FADE_MAX; // Set to max
    fadeAmount = -abs(fadeAmount); // Reverse direction to decrease
  } 
  if (period <= FADE_MIN) {
    period = FADE_MIN; // Set to min
    fadeAmount = abs(fadeAmount); // Reverse direction to increase
  }

  // Optional: Add a small delay to smooth out the transition
  delay(looptime);
}