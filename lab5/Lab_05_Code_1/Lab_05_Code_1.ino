#include <Arduino.h>        // core functions of Arduino 
#include <avr/io.h>         // direct register access, as PINB, DDRB and so on

#define LED_PIN PB5          // LED connected to pin 13 = PB5
#define BUTTON_PIN PB0       // button connected to pin 8 = PB0

// extern "C" is needed to prevents name corruption, so assembly will find them
extern "C" void modeA();
extern "C" void modeB();
extern "C" void modeC();

extern "C" void modeA_entry();
extern "C" void modeB_entry();
extern "C" void modeC_entry();

volatile uint8_t action = 0;  // global state

// action functions - each action defines a different LED bahavior
void action0() {
  digitalWrite(LED_PIN, HIGH);// LED ON
  delay(200);
  digitalWrite(LED_PIN, LOW); // LED OFF
  delay(200);
}

void action1() {
  digitalWrite(LED_PIN, LOW); // first LED OFF
  delay(200);
  digitalWrite(LED_PIN, HIGH);// second LED ON (reversed)
  delay(200);
}

void action2() {
  digitalWrite(LED_PIN, HIGH);// LED ON
  delay(200);
  digitalWrite(LED_PIN, LOW); // LED OFF longer
  delay(400);
}

void action3() {
  action = 0;                 // reset back to first action
}

// jump table for IJMP
void (*actionTable[4])() = {
  action0,
  action1,
  action2,
  action3
};
// dispatch - executes function using its address (via Z register - r31:r30)
void dispatchAction() {
  void (*handler)() = actionTable[action];

  asm volatile(
    "movw r30,%A0\n"           // r30 low byte, r31 high byte
    "ijmp\n"                   // jump to address in Z
    :
    : "r"(handler)
  );
}

/*
  AVR is 8-bit → address is 16-bit → split into:
  r30 = low byte
  r31 = high byte
  r30+r31 → Z register → IJMP
*/
// wait for first press
void waitForFirstPress() {
  asm volatile(
    "boot_wait:\n"              // lable, the start of loop
    "sbic %[pin],0\n"           // skip next if bit is 0
    "rjmp boot_wait\n"          // not pressed, loop again
    :
    : [pin] "I" (_SFR_IO_ADDR(PINB)) // direct access to PINB
  );
}

// mode selection, counts number of presses in 2 sec
int chooseMode() {
  int presses = 0;
  unsigned long start = millis(); // start time

  while (millis() - start < 2000) { // 2 sec window
    if (digitalRead(BUTTON_PIN) == LOW) { // button pressed

      presses++;

      while (digitalRead(BUTTON_PIN) == LOW); // wait till release
      delay(50);
    }
  }

  if (presses < 1) presses = 1;
  if (presses > 3) presses = 3;

  return presses;
}

// modes
// modeA - normal
extern "C" void modeA() {
  while (1) {
    if (digitalRead(BUTTON_PIN) == LOW) {

      action++;
      if (action > 3) action = 0;

      while (digitalRead(BUTTON_PIN) == LOW);
      delay(50);
    }

    dispatchAction();
  }
}
// modeB - double
extern "C" void modeB() {
  while (1) {
    if (digitalRead(BUTTON_PIN) == LOW) {

      action++;
      if (action > 3) action = 0;

      while (digitalRead(BUTTON_PIN) == LOW);
      delay(50);
    }

    dispatchAction();
    dispatchAction();
    delay(400);         // pause between double blinks
  }
}
// modeC - direct fast blinking
extern "C" void modeC() {
  while (1) {
    digitalWrite(LED_PIN, HIGH);
    delay(40);
    digitalWrite(LED_PIN, LOW);
    delay(40);
  }
}

void setup() {

  pinMode(LED_PIN, OUTPUT);             // LED as output
  pinMode(BUTTON_PIN, INPUT_PULLUP);    // button pull-up

  waitForFirstPress();                  // wait till first press

  // start confirmation
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);

  int mode = chooseMode();              // detect number of presses

  // jump to selected mode
  if (mode == 1) {
    asm volatile("jmp modeA_entry");
  }

  if (mode == 2) {
    asm volatile("jmp modeB_entry");
  }

  if (mode == 3) {
    asm volatile("jmp modeC_entry");
  }

  while (1);                            // safety loop
}

//prevent optimizer from removing functions, needed cause assembly jumps require stable names
extern "C" __attribute__((used)) void modeA_entry() {
  modeA();
}

extern "C" __attribute__((used)) void modeB_entry() {
  modeB();
}

extern "C" __attribute__((used)) void modeC_entry() {
  modeC();
}

void loop() {}                           // loop is empty, everything is done by interrupts
