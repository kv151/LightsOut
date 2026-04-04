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
#define P1BUTTONPIN 2 // p1's push button
#define P2BUTTONPIN 3 // p2's push button
#define P1LEDPIN 5 // pins for green win led
#define P2LEDPIN 10 
#define P1JUMPLEDPIN 8 //pins for yellow jumpstart leds
#define P2JUMPLEDPIN 9
/*--------------GAME STATUS---------------*/
volatile bool gameRunning = false;
volatile bool buttonPressedP1 = false;
volatile bool buttonPressedP2 = false;
volatile bool jumpStartP1 = false;
volatile bool jumpStartP2 = false;
char winner = 0;
/*------------------ Clocks and timers--------------------*/
volatile unsigned int lightsOutClock = 0;
volatile unsigned int reactionTimeP1 = 0;
volatile unsigned int reactionTimeP2 = 0;
#define STARTSEQUENCEDELAY 1000 //delay before start sequence initated
int lightsOutDelay = random(200,3000); //random delay time for lights out between 0.2 and 3 seconds.

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

    attachInterrupt(digitalPinToInterrupt(P1BUTTONPIN), p1ButtonISR, FALLING);      //ISR attached to button presses to allow game to keep running in loop and for accurate time detection
    attachInterrupt(digitalPinToInterrupt(P2BUTTONPIN), p2ButtonISR, FALLING);
}

void loop() {
    //Start a new game
    gameRunning = true;
    lightsOutDelay = random(200, 3000);
    if (winner != 0) {
        buttonPressedP1 = false;
        buttonPressedP2 = false;
        jumpStartP1 = false;
        jumpStartP2 = false;
        // game loop
        delay(STARTSEQUENCEDELAY); 
        
        lightpattern();
        
        lightsOutClock = millis();
        #ifdef DEBUG
        Serial.println("Lights out clock");
        Serial.print(lightsOutClock);
        #endif
        
        //check for jump start
        
        //ISR for buttons
        
        
    }
    #ifdef DEBUG
    Serial.println("Reaction time:");
    Serial.print(reactionTimeP1);
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

/*****
Purpose: ISR for p1 button
*****/
void p1ButtonISR(){
    if (gameRunning && !buttonPressedP1) {
        reactionTimeP1 = millis() - lightsOutClock;
        buttonPressedP1 = true;
    }
}
/*****
Purpose: ISR for p2 button
*****/
void p2ButtonISR()
{
    if (gameRunning && !buttonPressedP2)
    {
        reactionTimeP2 = millis() - lightsOutClock;
        buttonPressedP2 = true;
    }
}
