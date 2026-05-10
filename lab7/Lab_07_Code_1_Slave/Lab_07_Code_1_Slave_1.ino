// Slave Code
#include <avr/io.h>
#include <Arduino.h>

void SPI_SlaveInit() {                  // function for initializing SPI as slave
  DDRB |= (1 << PB4);                   // configure PB4 = MISO as output, because slave sends data to master

  DDRB &= ~((1 << PB2) | (1 << PB3) | (1 << PB5)); // configure PB2 = SS, PB3 = MOSI, PB5 = SCK as inputs

  SPCR = (1 << SPE);                    // enable SPI in slave mode

  SPDR = 60;                            // put 60 into SPDR, so slave can send it to master
}

void setup() {
  Serial.begin(9600);                   // serial communication at 9600 baud
  SPI_SlaveInit();                      // call function to initialize SPI slave
}

void loop() {
  if (SPSR & (1 << SPIF)) { // SPI Interrupt Flag
    uint8_t received = SPDR;   // reading SPDR clears SPIF after SPSR was read

    Serial.println(received);           // print received byte from master
  }

  SPDR = 60;                            // load 60 again into SPDR for next SPI transfer
}
