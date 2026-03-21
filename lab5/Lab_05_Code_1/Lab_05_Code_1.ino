#include <Arduino.h>           // core functions of Arduino 
#include <avr/io.h>            // direct register access, as PINB, DDRB and so on

#define LED_PIN PB5            // LED connected to pin 13 = PB5
#define BUTTON_PIN PB0         // button connected to pin 8 = PB0

// extern "C" is needed to prevents name corruption, so assembly will find them
extern "C" void modeA();       // declare modeA() with C link
extern "C" void modeB();       // declare modeB() with C link
extern "C" void modeC();       // declare modeC() with C link

extern "C" void modeA_entry(); // declare wrapper for modeA()
extern "C" void modeB_entry(); // declare wrapper for modeB()
extern "C" void modeC_entry(); // declare wrapper for modeC()

volatile uint8_t action = 0;   // global state, 8-bit unsigned, volatile - not optimize it

// action functions - each action defines a different LED bahavior
void action0() {              // action 0 - normal blinking 
  digitalWrite(LED_PIN, HIGH);// LED ON
  delay(200);                 // wait 200 ms
  digitalWrite(LED_PIN, LOW); // LED OFF
  delay(200);                 // wait 200 ms
}

void action1() {              // action 1 - reversed blinking 
  digitalWrite(LED_PIN, LOW); // first LED OFF
  delay(200);                 // wait 200 ms
  digitalWrite(LED_PIN, HIGH);// second LED ON (reversed)
  delay(200);                 // wait 200 ms
}

void action2() {              // action 2 - blinking with longer OFF 
  digitalWrite(LED_PIN, HIGH);// LED ON
  delay(200);                 // wait 200 ms
  digitalWrite(LED_PIN, LOW); // LED OFF longer
  delay(400);                 // wait 400 ms
}

void action3() {              // action 3 - reset sequence
  action = 0;                 // reset back to first action
}

// jump table for IJMP
void (*actionTable[4])() = {  // array with 4 pointes, each is a function
  action0,                    // entry 0 points to action0()
  action1,                    // entry 1 points to action1()
  action2,                    // entry 2 points to action2()
  action3                     // entry 3 points to action3()
};
// dispatch - executes function using its address (via Z register - r31:r30)
void dispatchAction() {        // jumps to the action using IJMP 
  void (*handler)() = actionTable[action]; // select the function pointer from table

  asm volatile(                // start AVR assembly; volatile is used for not optimizing it
    "movw r30,%A0\n"           // r30 low byte, r31 high byte; move 16bit address into Z 
    "ijmp\n"                   // jump to address in Z
    :                          // no output 
    : "r"(handler)             // input operand - address in a register pair
  );
}

/*
  AVR is 8-bit → address is 16-bit → split into:
  r30 = low byte
  r31 = high byte
  r30+r31 → Z register → IJMP
*/
// waiting in assembly till button press
void waitForFirstPress() {
  asm volatile(                 // start AVR assembly; volatile is used for not optimizing it
    "boot_wait:\n"              // lable, the start of loop
    "sbic %[pin],0\n"           // skip next if bit is 0
    "rjmp boot_wait\n"          // not pressed, loop again
    :                           // no output
    : [pin] "I" (_SFR_IO_ADDR(PINB)) // input operand - direct access to PINB
  );
}

// mode selection, counts number of presses in 2 sec
int chooseMode() {
  int presses = 0;                // variable to store number of presses
  unsigned long start = millis(); // start time

  while (millis() - start < 2000) { // 2 sec window
    if (digitalRead(BUTTON_PIN) == LOW) { // check if button pressed

      presses++;                          // increase counter of presses

      while (digitalRead(BUTTON_PIN) == LOW); // wait till release
      delay(50);                              // small debounce delay
    }
  }

  if (presses < 1) presses = 1;           // if no press, then choose mode 1
  if (presses > 3) presses = 3;           // if more than 3 presses, then choose mode 3

  return presses;                         // return mode number
}

// modes
// modeA - normal
extern "C" void modeA() {        // mode A with C linkage
  while (1) {                    // infinite loop
    if (digitalRead(BUTTON_PIN) == LOW) {     // if button pressed

      action++;                               // move to next action
      if (action > 3) action = 0;             // go to 0 if action goes above 3

      while (digitalRead(BUTTON_PIN) == LOW); // wait until button released
      delay(50);                              // small debounce delay
    }

    dispatchAction();                         // execute action with indirect jump
  }
}
// modeB - double
extern "C" void modeB() {        // mode B with C linkage
  while (1) {                    // infinite loop
    if (digitalRead(BUTTON_PIN) == LOW) {     // if button pressed

      action++;                               // move to next action
      if (action > 3) action = 0;             // go to 0 if action goes above 3

      while (digitalRead(BUTTON_PIN) == LOW); // wait until button released
      delay(50);                              // small debounce delay
    }

    dispatchAction();                         // execute action with indirect jump (first time)
    dispatchAction();                         // execute action with indirect jump (second time)
    delay(400);                               // pause between double blinks - 400 ms
  }
}
// modeC - direct fast blinking
extern "C" void modeC() {        // mode C with C linkage
  while (1) {
    digitalWrite(LED_PIN, HIGH); // LED ON
    delay(40);                   // wait 40 ms
    digitalWrite(LED_PIN, LOW);  // LED OFF
    delay(40);                   // wait 40 ms
  }
}

void setup() {

  pinMode(LED_PIN, OUTPUT);             // LED as output
  pinMode(BUTTON_PIN, INPUT_PULLUP);    // button pull-up as input

  waitForFirstPress();                  // wait till first press

  // start confirmation
  digitalWrite(LED_PIN, HIGH);          // turn LED ON
  delay(200);                           // wait 200 ms 
  digitalWrite(LED_PIN, LOW);           // turn LED OFF

  int mode = chooseMode();              // detect number of presses

  // jump to selected mode
  if (mode == 1) {                      // if mode 1 selected
    asm volatile("jmp modeA_entry");    // absolute jump to modeA-entry
  }

  if (mode == 2) {                      // if mode 2 selected
    asm volatile("jmp modeB_entry");    // absolute jump to modeB-entry
  }

  if (mode == 3) {                      // if mode 3 selected
    asm volatile("jmp modeC_entry");    // absolute jump to modeC-entry
  }

  while (1);                            // safety loop
}

//prevent optimizer from removing functions, needed cause assembly jumps require stable names
extern "C" __attribute__((used)) void modeA_entry() { // entry wrapper for modeA(), compiler keeps it, and state=used
  modeA();                                            // call modeA()
}

extern "C" __attribute__((used)) void modeB_entry() { // entry wrapper for modeB(), compiler keeps it, and state=used
  modeB();                                            // call modeB()
}

extern "C" __attribute__((used)) void modeC_entry() { // entry wrapper for modeC(), compiler keeps it, and state=used
  modeC();                                            // call modeC()
}

void loop() {}                           // loop is empty, everything is done by interrupts
