// Board A - Master - Register-Level I2C / TWI
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#define SLAVE_ADDR 0x08                  // I2C address of Board B

#define BUTTON_A PD2                     // Arduino D2 = PD2

#define LED_A PB5                        // Arduino D13 onboard LED

#define I2C_SPEED 100000UL               // 100 kHz I2C speed; use 400000UL for 400 kHz

void TWI_init() {
  TWSR = 0x00;                           // set prescaler to 1
  TWBR = ((F_CPU / I2C_SPEED) - 16) / 2; // set I2C clock speed using TWBR formula; 72 for 100kHz, 12 for 400kHz

  TWCR = (1 << TWEN);                    // enable TWI hardware
  PORTC |= (1 << PC4) | (1 << PC5);      // enable internal pull-ups on SDA/SCL; PC4 = A4 = SDA, PC5 = A5 = SCL
}

void TWI_wait() {
  while (!(TWCR & (1 << TWINT)));        // wait until current TWI operation finishes
}

void TWI_start(uint8_t address) {
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // send START condition
  TWI_wait();                            // wait until START is completed

  TWDR = address;                        // load slave address with read/write bit into TWDR
  TWCR = (1 << TWINT) | (1 << TWEN);     // send slave address
  TWI_wait();                            // wait until address is sent
}

void TWI_stop() {
  TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // send STOP condition
}

void TWI_write(uint8_t data) {
  TWDR = data;                           // load data byte into data register
  TWCR = (1 << TWINT) | (1 << TWEN);     // send data byte
  TWI_wait();                            // wait until data is sent
}

uint8_t TWI_read_NACK() {
  TWCR = (1 << TWINT) | (1 << TWEN);     // read one byte and send NACK after it
  TWI_wait();                            // wait until byte is received

  return TWDR;                           // return received byte
}

void send_to_slave(uint8_t data) {
  TWI_start((SLAVE_ADDR << 1) | 0);      // start communication in write mode
  TWI_write(data);                       // send one protocol byte: 0x01 or 0x00
  TWI_stop();                            // finish communication
}

uint8_t request_from_slave() {
  uint8_t data;                          // variable for storing received byte from slave

  TWI_start((SLAVE_ADDR << 1) | 1);      // start communication in read mode
  data = TWI_read_NACK();                // read one byte from slave
  TWI_stop();                            // finish communication

  return data;                           // return received byte
}

void setup() {
  DDRD &= ~(1 << BUTTON_A);              // set Button A as input
  PORTD |= (1 << BUTTON_A);              // enable internal pull-up for Button A; not pressed = HIGH, pressed = LOW
  DDRB |= (1 << LED_A);                  // set LED A as output

  TWI_init();                            // start I2C/TWI as master
}

void loop() {
  if (!(PIND & (1 << BUTTON_A))) {       // check if Button A is pressed
    send_to_slave(0x01);                 // tell slave: LED ON
  } else {                               // if Button A is not pressed
    send_to_slave(0x00);                 // tell slave: LED OFF
  }

  uint8_t received = request_from_slave(); // ask slave for Button B state and store answer

  if (received == 0x02) {                // if slave sends 0x02, Button B is pressed
    PORTB |= (1 << LED_A);               // LED A ON
  } else {                               // if slave sends other value, Button B is not pressed
    PORTB &= ~(1 << LED_A);              // LED A OFF
  }

  _delay_ms(10);                         // small delay for stable communication
}
