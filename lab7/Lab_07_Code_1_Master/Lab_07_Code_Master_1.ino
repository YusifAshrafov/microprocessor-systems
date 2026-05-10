// Master Code
#include <avr/io.h>
#include <util/delay.h>

uint8_t dataSequence[3] = {85, 170, 255}; // array with 3 values that master sends to slave
uint8_t indexTx = 0;                      // stores current index of dataSequence

void SPI_MasterInit() {                   // function for initializing SPI as master
  DDRB |= (1 << PB2) | (1 << PB3) | (1 << PB5); // PB2 = SS, PB3 = MOSI, PB5 = SCK as outputs

  DDRB &= ~(1 << PB4);                    // PB4 = MISO as input

  PORTB |= (1 << PB2);                    // set SS HIGH, so slave is not selected

  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0); // enable SPI, master mode, prescaler 128, fosc/128 = 16MHz/128 = 125kHz
}

uint8_t SPI_MasterTransmit(uint8_t data) { // function for sending one byte by SPI
  PORTB &= ~(1 << PB2);                   // set SS LOW to select slave

  SPDR = data;                            // put data into SPI Data Register to start transmission

  while (!(SPSR & (1 << SPIF))) {         // wait until SPIF flag becomes 1
  }

  uint8_t received = SPDR;                // read received byte - SPDR from slave and clear SPIF flag

  PORTB |= (1 << PB2);                    // set SS HIGH to deselect slave

  return received;                        // return received data from slave
}

void setup() {
  SPI_MasterInit();                       // call function to initialize SPI master
}

void loop() {
  SPI_MasterTransmit(dataSequence[indexTx]); // send current value from dataSequence to slave

  indexTx++;                              // move to next value
  if (indexTx >= 3) {                     // check if index goes out of array
    indexTx = 0;                          // reset index back to first value
  }

  _delay_ms(1000);                        // wait 1 second before sending next value
}
