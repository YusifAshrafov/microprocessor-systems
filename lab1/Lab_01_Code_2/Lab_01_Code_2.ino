// LED blink code using registers
void setup() {                  // Runs once when the board starts
  DDRB |= (1 << DDB5);          // Set the PB5 pin as OUTPUT
}

void loop() {                   // Runs repeatedly
  PORTB |= (1 << PORTB5);       // Turn LED ON by setting PB5 HIGh
  delay(500);                   // Wait for 500 milliseconds
  PORTB &= ~(1 << PORTB5);      // Turn LED OFF by setting PB5 LOW
  delay(500);                   // Wait for 500 milliseconds
  //PORTB ^= (1 << PORTB5);     // Toggle PB5 instead of using separate ON/OFF writes
}

// delay(500); - Comment this out to test flashing as quickly as possible
