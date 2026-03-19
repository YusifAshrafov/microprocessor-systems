// EEPROM (Electrically Erasable Programmable Read-Only Memory)
// keeps data even after power is off
#include <Arduino.h>                        // needed for serial, setup, loop and delay
#include <avr/io.h>                         // AVR registers: EECR, EEAR, EEDR, EEPR, EEMPE, EERE

volatile uint8_t counter = 0;               // 8-bit unsigned counter in SRAM; volatile - not optimize it

void setup() {
  Serial.begin(9600);                       // serial communication at 9600 baud

  while (EECR & (1 << EEPE)) {}             // wait while EEPROM is busy writing 
  EEAR = 0x00;                              // select EEPROM 0x00 address
  EECR |= (1 << EERE);                      // set EERE in EECR to control read
  counter = EEDR;                           // copy read byte from EEDR into counter

  Serial.println("Commands: S/s = save, R/r = reset");
}

void loop() {
  asm volatile(                             // start AVR assembly; volatile is used for not optimizing it
    "lds r24, counter  \n"                  // load direct from SRAM, load counter into r24
    "inc r24           \n"                  // increment r24 by 1
    "sts counter, r24  \n"                  // store direct to SRAM, store r24 into counter
    :                                       // no output operands
    :                                       // no input operands
    : "r24"                                 // clobber list - this register is modified
  );

  Serial.print("Counter = ");
  Serial.println(counter);

  if (Serial.available()) {
    char command = Serial.read();           // read one character from buffer
    // SAVE (S or s)
    if (command == 'S' || command == 's') { // if s/S is typed, then do:
      while (EECR & (1 << EEPE)) {}         // wait while EEPROM is busy writing 
      EEAR = 0x00;                          // select EEPROM 0x00 address
      EEDR = counter;                       // put counter into EEDR
      EECR |= (1 << EEMPE);                 // set Master Program Enable
      EECR |= (1 << EEPE);                  // set Program Enable
      Serial.println("Store");
    }

    // RESET (R or r)
    if (command == 'R' || command == 'r') { // if r/R is typed, then do:
      counter = 0;                          // reset after reboot
      while (EECR & (1 << EEPE)) {}         // wait while EEPROM is busy writing 
      EEAR = 0x00;                          // select EEPROM 0x00 address
      EEDR = 0;                             // put zero into EEDR
      EECR |= (1 << EEMPE);                 // set for unlocking write sequence
      EECR |= (1 << EEPE);                  // set it to write zero into EEPROM
      Serial.println("Reset");
    }
  }

  delay(1000);                              // wait 1 second before next loop
}
