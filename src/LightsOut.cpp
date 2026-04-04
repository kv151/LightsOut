/* Lights Out - A 2 player reaction test game */
#include <Arduino.h>
#include<Wire.h>

#define DEBUG

//Define global definitions here:
/* ------------ SHIFT REGISTER -------------*/
#define SRDATAPIN 4 //shift register serial pin
#define SRCLKPIN 6 //serial clock pin
#define SRLATCHPIN 7 //storage register clock pin (latch)
byte pattern = 0;
/* ------------ GAME LEDS & BUTTON CONNECTIONS-----------------------*/
#define P1BUTTONPIN 2
#define P2BUTTONPIN 3
#define P1LEDPIN 5 // pin for led
#define P2LEDPIN 10
#define P1JUMPLEDPIN 8
#define P2JUMPLEDPIN 9
/*--------------LIGHTS STATUS---------------*/
#define LIGHTSOFF  0
#define FIVEREDLIGHTS  1
#define LIGHTSOUT 2
int gameStatus = LIGHTSOFF; // state variable to track the game status so that the reset is only triggered when the game has ended and to display the corect message when a jump start occurs
float reactionTime;
int buttonState1 = HIGH; //button active low
int buttonState2 = HIGH;
char winner;
int startTime = millis();

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
    reactionTime = 0;
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

    buttonState1 = digitalRead(P1BUTTONPIN);
    buttonState2 = digitalRead(P2BUTTONPIN);

    /*read winning player - set up for one player rn*/
    if (buttonState1 == LOW) {               //p1 button pressed (active low)
        winner = 1;
        digitalWrite(P1LEDPIN,HIGH);
        reactionTime = millis() - reactionTime - startTime;         //reaction time calucaltion between now and lights out
        
    } else if (buttonState2 == LOW) {
        winner = 2;
        digitalWrite(P2LEDPIN, HIGH);
        reactionTime = millis() - reactionTime - startTime;
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
      Serial.println(pattern, BIN);
      Serial.println("");
  #endif
      digitalWrite(SRLATCHPIN, LOW); // prepare to shift out from register
      shiftOut(SRDATAPIN, SRCLKPIN, MSBFIRST, pattern);
      digitalWrite(SRLATCHPIN, HIGH); // latch data to output
      delay(1000);                    // 1 second delay betwwen lightup as per f1 standard
    }
}