const byte ZC_PIN = 2;  // Zero-crossing detector input
const byte TRIAC_PINS[] = {3, 4, 5, 6, 7, 8, 9, 10}; // 8 output channels
const unsigned long PULSE_WIDTH = 30; // TRIAC pulse width in microseconds
const unsigned long HALF_CYCLE = 8333; // 8.33ms for 60Hz AC

// DIP switch pins
const byte DIP_PINS[] = {A0, A1, A2, A3, A4}; // 5 DIP switches

volatile unsigned long zcTime = 0;
volatile bool zcFlag = false;
unsigned long channelDelays[8];
int fadeSpeeds[8]; // Fade speed based on pattern
int currentBrightness[8];
bool triacState[8];

// Predefined patterns
const int dimmingPatterns[4][8] = {
    {300, 500, 700, 900, -300, -500, -700, -900}, // Pattern 1
    {400, 600, 800, 1000, -400, -600, -800, -1000}, // Pattern 2
    {200, 300, 400, 500, -200, -300, -400, -500}, // Pattern 3
    {100, 200, 300, 400, -100, -200, -300, -400}  // Pattern 4
};

const bool relayPatterns[4][8] = {
    {1, 0, 1, 0, 1, 0, 1, 0}, // Pattern 1 (Alternate ON/OFF)
    {0, 1, 0, 1, 0, 1, 0, 1}, // Pattern 2 (Opposite Alternate)
    {1, 1, 0, 0, 1, 1, 0, 0}, // Pattern 3 (Pairs ON/OFF)
    {0, 0, 1, 1, 0, 0, 1, 1}  // Pattern 4 (Inverse Pairs)
};

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

void readDIPSwitch() {
    int mode = digitalRead(DIP_PINS[0]); // Mode selection
    int pattern = (digitalRead(DIP_PINS[1]) << 3) | 
                  (digitalRead(DIP_PINS[2]) << 2) | 
                  (digitalRead(DIP_PINS[3]) << 1) | 
                  (digitalRead(DIP_PINS[4]) << 0); // Convert 4-bit input

    int patternCount = 0;
    for (int i = 1; i <= 4; i++) {
        if (digitalRead(DIP_PINS[i]) == LOW) patternCount++; // Count ON switches
    }

    if (patternCount > 1) { // Invalid selection (more than one switch ON)
        handleInvalidSelection();
        return;
    }

    if (mode == LOW) { // Dimming Mode
        if (pattern >= 1 && pattern <= 4) {
            memcpy(fadeSpeeds, dimmingPatterns[pattern - 1], sizeof(fadeSpeeds));
        }
    } else { // Relay Mode
        if (pattern >= 1 && pattern <= 4) {
            for (int i = 0; i < 8; i++) {
                digitalWrite(TRIAC_PINS[i], relayPatterns[pattern - 1][i]);
            }
        }
    }
}

void handleInvalidSelection() {
    while (true) { // Infinite loop until DIP switch is corrected
        for (byte i = 0; i < 8; i++) {
            digitalWrite(TRIAC_PINS[i], LOW);
        }
        delay(2000);
        for (byte i = 0; i < 8; i++) {
            digitalWrite(TRIAC_PINS[i], HIGH);
        }
        delay(2000);
    }
}

void setup() {
    pinMode(ZC_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(ZC_PIN), zcISR, RISING);

    for (byte i = 0; i < 8; i++) {
        pinMode(TRIAC_PINS[i], OUTPUT);
        currentBrightness[i] = 100; // Start with minimum brightness
    }

    for (byte i = 0; i < 5; i++) {
        pinMode(DIP_PINS[i], INPUT_PULLUP); // Set DIP switch as input with pull-up
    }

    readDIPSwitch();
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

    readDIPSwitch(); // Continuously check DIP switch
}
