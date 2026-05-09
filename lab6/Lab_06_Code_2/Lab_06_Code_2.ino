// 5 sec code
#include <avr/io.h>                     // access to AVR registers as DDRD, PORTD, DDRB, PORTB and others
#include <avr/interrupt.h>              // access to ISR and interrupt-related functions

volatile uint8_t count = 0;             // 8-bit unsigned counter in SRAM; volatile - not optimize it
volatile uint8_t is_running = 1;        // stores timer state; 1 = running, 0 = paused
volatile uint8_t seconds_passed = 0;    // stores how many seconds passed

// Bits: [g f e d c b a]
const uint8_t segments[10] = {           // array with segment patterns for digits 0-9
    0b00111111,                         // segments for digit 0
    0b00000110,                         // segments for digit 1
    0b01011011,                         // segments for digit 2
    0b01001111,                         // segments for digit 3
    0b01100110,                         // segments for digit 4
    0b01101101,                         // segments for digit 5
    0b01111101,                         // segments for digit 6
    0b00000111,                         // segments for digit 7
    0b01111111,                         // segments for digit 8
    0b01101111                          // segments for digit 9
};

void displayDigit(uint8_t digit) {      // function for showing digit on 7-segment display
    uint8_t val = segments[digit];      // get segment pattern for selected digit

    PORTD = (PORTD & 0x07) | ((val & 0x1F) << 3); // keep PD0-PD2 same, put a-e bits into PD3-PD7
    PORTB = (PORTB & 0xFC) | ((val >> 5) & 0x03); // keep PB2-PB7 same, put f-g bits into PB0-PB1
}

void timer1_start(void) {               // function for starting Timer1
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); // enable CTC mode and start timer with prescaler 1024
}

void timer1_stop(void) {                // function for stopping Timer1
    TCCR1B = (1 << WGM12);              // stop timer clock, but keep CTC mode
}

void setup() {                          // runs once when the board starts
    cli();                              // disable global interrupts during setup

    DDRD |= 0xF8;                       // configure PD3-PD7 as output

    DDRD &= ~(1 << PD2);                // configure PD2 as input
    PORTD |= (1 << PD2);                // enable internal pull-up on PD2, so pin is HIGH

    DDRB |= 0x03;                       // configure PB0-PB1 as output

    PORTD &= 0x07;                      // clear PD3-PD7, keep PD0-PD2 unchanged
    PORTB &= 0xFC;                      // clear PB0-PB1, keep PB2-PB7 unchanged

    TCCR1A = 0;                         // clear Control Register A
    TCCR1B = 0;                         // clear Control Register B
    TCNT1  = 0;                         // reset Timer1 counter

    OCR1A = 15624;                      // set compare value for 1 second delay

    TIMSK1 |= (1 << OCIE1A);            // enable Timer1 Compare Match A interrupt

    EICRA |= (1 << ISC01);              // set ISC01 bit
    EICRA &= ~(1 << ISC00);             // clear ISC00 bit
    EIFR  |= (1 << INTF0);              // clear pending INT0 flag
    EIMSK |= (1 << INT0);               // enable external interrupt INT0

    displayDigit(count);                // display initial digit 0

    timer1_start();                     // start Timer1
    sei();                              // enable global interrupts
}

ISR(TIMER1_COMPA_vect) {                // Interrupt Service Routine for Timer1 Compare Match A
    seconds_passed++;                   // increase passed seconds by 1

    if (seconds_passed >= 5) {          // check if 5 seconds passed
        seconds_passed = 0;             // reset seconds counter back to 0

        count++;                        // increase count by 1

        if (count > 9) {                // check if count goes above 9
            count = 0;                  // reset count back to 0
        }

        displayDigit(count);            // update 7-segment display with new count
    }
}

ISR(INT0_vect) {                        // Interrupt Service Routine for external interrupt - INT0
    is_running ^= 1;                    // toggle running state between 0 and 1

    if (is_running) {                   // if timer should run
        timer1_start();                 // start Timer1 again
    } else {                            // if timer should pause
        timer1_stop();                  // stop Timer1 and keep current counter value
    }
}

void loop() {                           // loop is empty, everything is done by interrupts
}
