/* Lights Out - A 2 player reaction test game */
#include <Arduino.h>
#include<Wire.h>
//Define global definitions here:
/* ------------ SHIFT REGISTER -------------*/
#define SRDATAPIN 3 //shift register serial pin
#define SRCLKPIN 6 //serial clock pin
#define SRLATCHPIN 7 //storage register clock pin (latch)
byte pattern = 0;
/* ------------ GAME LEDS -----------------------*/
#define P1LEDPIN 8 // pin for led
#define P1BUTTONPIN 2
#define P2BUTTON 3
void lightpattern();
/*--------------GAME STATUS---------------*/
#define ENDED  0
#define PLAYING  1
#define JUMPSTART 2
int gameStatus = ENDED; // state variable to track the game status so that the reset is only triggered when the game has ended and to display the corect message when a jump start occurs
long int gameDelay = 0;
float reactionTime;
int buttonState = HIGH; //button active low

long startTime = millis(); //start a clock ticking
float randomdelay = random(200,3000); //random delay time for lights out between 0.2 and 3 seconds.
//put function declarations here:
void lightpattern();

/*=========================MAIN ARDUINO CODE===========================================================*/
void setup() {
    pinMode(P1LEDPIN, OUTPUT);
    pinMode(P1BUTTONPIN, INPUT_PULLUP); // use an intenral pullup no resistor needed on the button
    #ifdef DEBUG
        Serial.begin(9600);
    #endif
    // set lights as output
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
      pattern = pattern | (1 << i); // lights up each light in sequence by ORing each bit with the previous one shifted per second
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