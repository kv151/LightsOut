/* Lights Out - A 2 player reaction test game */
#include <Arduino.h>
#include<Wire.h>
//Define global definitions here:
/* ------------ SHIFT REGISTER -------------*/
#define SRDATAPIN 3 //shift register serial pin
#define SRCLKPIN 6 //serial clock pin
#define SRLATCHPIN 7 //storage register clock pin (latch)
byte pattern = 0;
/* ------------ GAME LEDS & BUTTON CONNECTIONS-----------------------*/
#define P1LEDPIN 8 // pin for led
#define P2LEDPIN 9
#define P1JUMPLEDPIN 6
#define P2JUMPLEDPIN 7
#define P1BUTTONPIN 2
#define P2BUTTONPIN 3
/*--------------LIGHTS STATUS---------------*/
#define LIGHTSOFF  0
#define FIVEREDLIGHTS  1
#define LIGHTSOUT 2
int gameStatus = LIGHTSOFF; // state variable to track the game status so that the reset is only triggered when the game has ended and to display the corect message when a jump start occurs
float reactionTime;
int buttonState = HIGH; //button active low

int lightsOutDelay = random(200,3000); //random delay time for lights out between 0.2 and 3 seconds.
#define STARTSEQUENCEDELAY 1000 //delay before start sequence initated

//put function declarations here:
void lightpattern();

/*=========================MAIN ARDUINO CODE===========================================================*/
void setup() {
    pinMode(P1BUTTONPIN, INPUT_PULLUP); // use an intenral pullup no resistor needed on the button
    pinMode(P2BUTTONPIN,INPUT_PULLUP);
    #ifdef DEBUG
    Serial.begin(9600);
    #endif

    pinMode(P1LEDPIN, OUTPUT);
    pinMode(P2LEDPIN,OUTPUT);
    pinMode(P1JUMPLEDPIN,OUTPUT);
    pinMode(P2JUMPLEDPIN, OUTPUT);

}

void loop() {
    /* light sequence */
    gameStatus = LIGHTSOFF;
    delay(STARTSEQUENCEDELAY);                          //delay before start sequence
    lightpattern();                                     //start sequence - after this 5 red lights
        gameStatus = FIVEREDLIGHTS;
    delay(lightsOutDelay);
    pattern = 0;
    digitalWrite(SRLATCHPIN, LOW);                      // prepare to shift out from register
    shiftOut(SRDATAPIN, SRCLKPIN, MSBFIRST, pattern);   // turn off lights
    digitalWrite(SRLATCHPIN, HIGH);                     // latch data to output
        gameStatus = LIGHTSOUT;
        reactionTime = millis();

    /*read winning player - set up for one player rn*/
    if (digitalRead(P1BUTTONPIN) == 0 ) {               //p1 button pressed (active low)
        digitalWrite(P1LEDPIN,HIGH);
        long stopTime = millis();
        reactionTime = millis() - reactionTime;         //reaction time calucaltion between now and lights out
        
    }

    #ifdef DEBUG
    Serial.println("Reaction time:");
    Serial.print(reactionTime);
    Serial.print(" ms");
    #endif
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