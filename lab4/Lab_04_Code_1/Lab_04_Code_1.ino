#include <avr/io.h>                     // access to AVR registers as DDRB, PORTB, TCCR1B and others
#include <avr/interrupt.h>              // access to ISR and other interrupt-related functions
#define PULSE_PIN PB5                   // port b 5 as the output pin - D13
#define BUTTON_BIT PD2                  // port d 2 as the input pin - D2

volatile uint8_t countdown = 0;         // 8-bit unsigned counter in SRAM; volatile - not optimize it

ISR(INT0_vect) {                        // Interrupt Service Routine for external interrupt - INT0
    PORTB |= (1 << PULSE_PIN);          // set PB5 high; turn LED ON
    countdown = 10;                     // load countdown with 10 (before turning off)
    TCNT1 = 0;                          // reset Timer1 counter to start timing from zero
    TIMSK1 |= (1 << OCIE1A);            // enable Timer1 Compare Match A interrupt
}

// runs every 1 ms
ISR(TIMER1_COMPA_vect) {                // Interrupt Service Routine for Timer1 Compare Match A
    if (countdown > 0) {                // check if pulse is active
        countdown--;                    // decrease pulse by 1
        if (countdown == 0) {           // check if 10 ms has finished 
            PORTB &= ~(1 << PULSE_PIN); // clear PB5; turn LED OFF
            TIMSK1 &= ~(1 << OCIE1A);   // disable Timer1 Comare Match A interrupt
        }
    }
}

void setup() {
    DDRB |= (1 << PULSE_PIN);           // configure PB5 as output
    PORTB &= ~(1 << PULSE_PIN);         // set PB5 LOW so output starts OFF

    DDRD &= ~(1 << BUTTON_BIT);         // configure PD2 as input
    PORTD |= (1 << BUTTON_BIT);         // enable internal pull-up on PD2, so pin is HIGH

    // INT0 is triggered on falling edge:
    EICRA &= ~((1 << ISC00));           // clear ISC00 bit
    EICRA |=  (1 << ISC01);             // set ISC bit

    EIMSK |= (1 << INT0);               // enable external interrupt INT0

    TCCR1A = 0;                         // clear Control Register A
    TCCR1B = 0;                         // clear Control Register B
    TCNT1 = 0;                          // reset counter

    OCR1A = 249;                        // set compare value to 249
    // CTC - clear timer on compare match
    TCCR1B |= (1 << WGM12);             // enable CTC mode, counts till OCR1A, then resets
    TIMSK1 |= (1 << OCIE1A);            // enable Compare Match A interrupt

    TCCR1B |= (1 << CS11) | (1 << CS10);// set prescaler to 64 and start Timer1 
    
    sei();                              // enable global interrupts
}

void loop() {                           // loop is empty, everything is done by interrupts
}
