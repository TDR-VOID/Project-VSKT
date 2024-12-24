#include <TimerOne.h>

const byte INTERRUPT_PIN = 2;
const byte TRIAC_PIN = 3;
const byte TRIAC_PULSE_MICROS = 30;

const int FADE_MAX = 7000;
const int FADE_MIN = 10;
const int looptime = 5;
int fadeAmount = 10;

volatile bool triacOn;
volatile int period = FADE_MIN; // microseconds cut out from AC pulse



void zeroCrossing() {
  triacOn = false; // triac tuns off self at zero crossing
  Timer1.setPeriod(period); // to call triacPulse() after off period
}

void triacPulse() {
  if (triacOn) { // stop pulse
    digitalWrite(TRIAC_PIN, LOW);
    Timer1.stop();
  } else { // start pulse
    digitalWrite(TRIAC_PIN, HIGH);
    triacOn = true;
    Timer1.setPeriod(TRIAC_PULSE_MICROS);
  }
}

void setup() {
  pinMode(TRIAC_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), zeroCrossing, RISING);
  Timer1.initialize();
  Timer1.attachInterrupt(triacPulse);
}


void loop() {
  period = period + fadeAmount;
  fadeAmount = (period <= FADE_MIN || period >= FADE_MAX) ? -fadeAmount : fadeAmount;
  delay(looptime);
}


