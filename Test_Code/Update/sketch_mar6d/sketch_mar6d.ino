const byte ZC_PIN = 2;  // Zero-crossing detector input
const byte TRIAC_PINS[] = {3, 4, 5, 6, 7, 8, 9, 10}; // 8 output channels
const unsigned long PULSE_WIDTH = 30; // TRIAC pulse width in microseconds
const unsigned long HALF_CYCLE = 8333; // 8.33ms for 60Hz AC

volatile unsigned long zcTime = 0;
volatile bool zcFlag = false;
unsigned long channelDelays[8];
int fadeSpeeds[8] = {300, 500, 70, 90, -30, -50, -70, -900}; // Predefined fade speeds
int currentBrightness[8];
bool triacState[8];

void zcISR() {
  zcTime = micros();
  zcFlag = true;
  memset(triacState, 0, sizeof(triacState)); // Reset all TRIAC states
}

void updateChannels() {
  if (!zcFlag) return;
  
  unsigned long currentMicros = micros();
  unsigned long elapsed = currentMicros - zcTime;

  for (byte i = 0; i < 8; i++) {
    if (!triacState[i] && elapsed >= channelDelays[i]) {
      digitalWrite(TRIAC_PINS[i], HIGH);
      triacState[i] = true;
      channelDelays[i] = currentBrightness[i];
    }
    
    if (triacState[i] && (currentMicros - (zcTime + channelDelays[i])) >= PULSE_WIDTH) {
      digitalWrite(TRIAC_PINS[i], LOW);
    }
  }

  if (elapsed >= HALF_CYCLE) zcFlag = false;
}

void setup() {
  for (byte i = 0; i < 8; i++) {
    pinMode(TRIAC_PINS[i], OUTPUT);
    currentBrightness[i] = 100; // Start with minimum brightness
  }
  
  attachInterrupt(digitalPinToInterrupt(ZC_PIN), zcISR, RISING);
}

void loop() {
  static unsigned long lastUpdate = 0;
  updateChannels();

  if (millis() - lastUpdate >= 20) { // Update brightness every 20ms
    lastUpdate = millis();
    
    for (byte i = 0; i < 8; i++) {
      currentBrightness[i] += fadeSpeeds[i];
      
      // Constrain and reverse direction
      if (currentBrightness[i] >= 8000 || currentBrightness[i] <= 100) {
        fadeSpeeds[i] = -fadeSpeeds[i]; // Reverse direction
        currentBrightness[i] = constrain(currentBrightness[i], 100, 8000);
      }
    }
  }
}