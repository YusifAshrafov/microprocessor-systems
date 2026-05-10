#define F_CPU 16000000UL
#include <avr/io.h>

#define SLAVE_ADDR 0x08                 // I2C address of this slave board
#define LED_B PB5                       // Arduino D13 onboard LED
#define BUTTON_B PD2                    // Arduino D2 = PD2 button pin

volatile uint8_t state = 0x00;          // stores Button B state; 0x02 = pressed, 0x00 = not pressed/sent

void TWI_ACK_continue() {               // function for sending ACK and continuing TWI communication
  TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT); // enable TWI, send ACK, clear TWINT to continue
}

void setup() {
  TWAR = (SLAVE_ADDR << 1);             // set slave address to SLAVE_ADDR; shifted because bit 0 is TWGCE

  TWI_ACK_continue();                   // enable TWI, send ACK, and get ready for communication

  PORTC |= (1 << PC4) | (1 << PC5);   // enable internal pull-ups on SDA/SCL; PC4 = A4 = SDA, PC5 = A5 = SCL

  DDRB |= (1 << LED_B);                 // configure PB5 as output for onboard LED
  DDRD &= ~(1 << BUTTON_B);             // configure PD2 as input for button
  PORTD |= (1 << BUTTON_B);             // enable internal pull-up; not pressed = HIGH, pressed = LOW
}

void loop() {
  if (!(PIND & (1 << BUTTON_B))) {      // check if Button B is pressed
    state = 0x02;                       // save pressed state to send to master
  }

  if (TWCR & (1 << TWINT)) {            // check if TWI action is finished and needs service
    uint8_t status = TWSR & 0xF8;       // read TWI status and clear 3 LSB bits

    switch (status) {                   // check what kind of TWI event happened

      case 0x60:                        // received own address + Write from master
        TWI_ACK_continue();             // send ACK and continue communication
        break;

      case 0x80: {                      // received data after own address + Write
        uint8_t received = TWDR;        // read received byte from TWDR

        if (received == 0x01) {         // check if master command is LED ON
          PORTB |= (1 << LED_B);        // turn LED B ON
        } else {                        // if received command is not 0x01
          PORTB &= ~(1 << LED_B);       // turn LED B OFF
        }

        TWI_ACK_continue();             // send ACK and continue communication
        break;
      }

      case 0xA8:                        // received own address + Read from master
        TWDR = state;                   // put current button state into TWDR to send to master
        TWI_ACK_continue();             // send data and continue communication
        state = 0x00;                   // clear state after sending it
        break;

      default:                          // for other TWI status cases
        TWI_ACK_continue();             // send ACK to continue communication
        break;
    }
  }
}
