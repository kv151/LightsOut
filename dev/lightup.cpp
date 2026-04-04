/* LIGHTS OUT - A reaction speed game for 2 players inspired by Formula 1 */
#include <Arduino.h>
//Define global definitions here:
#define SRDATAPIN 3 //shift register serial pin
#define SRCLKPIN 6 //serial clock pin
#define SRLATCHPIN 7 //storage register clock pin (latch)

byte pattern = 0;
//put function declarations here:
void lightpattern();

void setup() {
  
}

void loop() {

  
}

//put function definitions here:
/*****
Purpose: Use shift register to light up 5 lights in a row with a one second delay
Parameter list:
  none
Return value:
  none
*****/
void lightpattern() {
    for (int i = 0; i < 5; i++) {
      pattern = pattern | (1 << i); // lights up each light in sequence by ORing each bit with the previous one
  #ifdef DEBUG
      Serial.print(pattern, BIN);
      Serial.println("");
  #endif
      digitalWrite(SRLATCHPIN, LOW); // prepare to shift out from register
      shiftOut(SRDATAPIN, SRCLKPIN, MSBFIRST, pattern);
      digitalWrite(SRLATCHPIN, HIGH); // latch data to output
      delay(1000);                    // 1 second delay betwwen lightup as per f1 standard
    }
}