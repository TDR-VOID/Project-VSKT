// --- Basic Test Code Traic

// Define the GPIO pin connected to the TRIAC
const int triacPin = 7; // You can change this to the desired GPIO pin

void setup() {
  // Set the GPIO pin as output
  pinMode(triacPin, OUTPUT);
}

void loop() {
  // Set the pin HIGH
  digitalWrite(triacPin, HIGH);
  delay(2000); // Wait for 2 seconds

  // Set the pin LOW
  digitalWrite(triacPin, LOW);
  delay(2000); // Wait for 2 seconds
}
