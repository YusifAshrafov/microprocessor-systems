// A = first number(operand); B = second number; R = result of subtraction; S = copy of SREG after subtraction
uint8_t A, B, R, S;                                         // uint8_t = 8-bit unsigned

void setup(){ 
  Serial.begin(9600);                                       // serial communication at 9600 baud
}

void loop(){
  if(!Serial.available()) {                                 // check if data arrived
    return;                                                 // if no, exit the loop
  }
  Serial.println("Enter first and second positive number(0-255):");

  A = Serial.parseInt();                                    // read first integer for serial input and store in A
  B = Serial.parseInt();                                    // read first integer for serial input and store in B
  while(Serial.available()) Serial.read();                  // clear char if remained from buffer
  
  asm volatile(                                             // start AVR assembly; volatile is used for not optimizing it
    "mov r16,%[a]\n"                                        // copy A into CPU r16 register
    "mov r17,%[b]\n"                                        // copy B into CPU r17 register
    "sub r16,r17\n"                                         // subtract r17 from r16 and store the result in r16, SREG is updated
    "in  r19,__SREG__\n"                                    // copy SREG status register into r19
    "mov %[r],r16\n"                                        // move result from r16 to into R (C variable)
    "mov %[s],r19\n"                                        // move SREG value from r19 to into S (C variable)
    : [r]"=r"(R), [s]"=r"(S)                                // output operands - write values into R and S
    : [a]"r"(A), [b]"r"(B)                                  // input operands - read values from A and B
    : "r16","r17","r19"                                     // clobber list - those registers are modified
  );
  Serial.print(" A=");Serial.print(A);                      // print A input number
  Serial.print(" B=");Serial.print(B);                      // print B input number
  Serial.print(" R=");Serial.print(R);                      // print Result(R) number
  Serial.print(" Z=");Serial.print((S>>1)&1);               // extract bit 1 of SREG and print Zero flag
  Serial.print(" C=");Serial.println(S&1);                  // extract bit 0 of SREG and print Carry flag
  Serial.print(" SREG=");
  for(int i=7; i>=0; i--) Serial.print((S>>i)&1);           // print SREG bit-by-bit
  Serial.println();
}
