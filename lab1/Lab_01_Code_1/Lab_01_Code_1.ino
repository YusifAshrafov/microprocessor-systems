int pin = 13;                   // Store LED pin number in a "pin"

void setup() {                  // Runs once when the board starts
  pinMode(pin, OUTPUT);         // Set the pin as OUTPUT
}

void loop() {                   // Runs repeatedly
  digitalWrite(pin, HIGH);      // Turn LED ON
  delay(500);                   // Wait for 500 milliseconds
  digitalWrite(pin, LOW);       // Turn LED OFF
  delay(500);                   // Wait for 500 millis
}

// delay(500); - Comment this out to test flashing as quickly as possible
