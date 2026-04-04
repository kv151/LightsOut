/* Lights Out - A 2 player reaction test game */
#include <Arduino.h>
#include <LCD_I2C.h>
#include <Wire.h>

#define DEBUG

//Define global definitions here:
/* ------------ SHIFT REGISTER -------------*/
#define SRDATAPIN 4         //shift register serial pin
#define SRCLKPIN 6          //serial clock pin
#define SRLATCHPIN 7        //storage register clock pin (latch)
byte pattern = 0;
/* ------------ GAME LEDS & BUTTON CONNECTIONS-----------------------*/
#define P1BUTTONPIN 2       // p1's push button
#define P2BUTTONPIN 3       // p2's push button
#define P1LEDPIN 5          // pins for green win led
#define P2LEDPIN 10 
#define P1JUMPLEDPIN 8      //pins for yellow jumpstart leds
#define P2JUMPLEDPIN 9
/*--------------GAME STATUS---------------*/
volatile bool gameRunning = false;
volatile bool buttonPressedP1 = false;
volatile bool buttonPressedP2 = false;
volatile bool jumpStartP1 = false;
volatile bool jumpStartP2 = false;
char winner = 0;

enum gamestate {LINEUP, STARTSEQUENCE, LIGHTSOUT, GAMEOVER};
gamestate currentState = LINEUP;

/*------------------ Clocks and timers--------------------*/
volatile unsigned int lightsOutClock = 0;
volatile unsigned int reactionTimeP1 = 0;
volatile unsigned int reactionTimeP2 = 0;
#define STARTSEQUENCEDELAY 1000                 //delay before start sequence initated
int lightsOutDelay = random(200,3000);          //random delay time for lights out between 0.2 and 3 seconds.

LCD_I2C lcd(0x27,16,2);                //set 16x2 LCD display - check if I2C address is correct

//put function declarations here:
void lightpattern();
void printWinMessage(char winner);
void p1ButtonISR();
void p2ButtonISR();

    /*=========================MAIN ARDUINO CODE===========================================================*/
    void setup() {
    pinMode(P1BUTTONPIN, INPUT_PULLUP); // use an intenral pullup no resistor needed on the button
    pinMode(P2BUTTONPIN, INPUT_PULLUP);
    #ifdef DEBUG
    Serial.begin(9600);
    #endif

    lcd.begin();
    lcd.backlight();
    lcd.print("Lights Out!");
    lcd.setCursor(0,1);
    lcd.print("A reaction game");
    
    pinMode(P1LEDPIN, OUTPUT);
    pinMode(P2LEDPIN,OUTPUT);
    pinMode(P1JUMPLEDPIN,OUTPUT);
    pinMode(P2JUMPLEDPIN, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(P1BUTTONPIN), p1ButtonISR, FALLING);      //ISR attached to button presses to allow game to keep running in loop and for accurate time detection
    attachInterrupt(digitalPinToInterrupt(P2BUTTONPIN), p2ButtonISR, FALLING);
}

void loop() {
    switch (currentState) {
        case LINEUP:
            lcd.noBacklight();
            lcd.clear();
            delay(50);
            lcd.backlight();
            delay(50);
            lcd.setCursor(0,0);
            lcd.print("Hold both button");
            lcd.setCursor(0,1);
            lcd.print("to continue");

            if (digitalRead(P1BUTTONPIN) == LOW && digitalRead(P2BUTTONPIN) == LOW) {      //game starts when both players press their buttons
                delay(1000); // short delay for reset
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Grid set");
                digitalWrite(P1LEDPIN, LOW);
                digitalWrite(P2LEDPIN, LOW);
                digitalWrite(P1JUMPLEDPIN, LOW);
                digitalWrite(P2JUMPLEDPIN, LOW); 
                currentState = STARTSEQUENCE;
            }
            break;
        
        case STARTSEQUENCE:
            lcd.clear();
            lcd.noBacklight();
        

            lightsOutDelay = random(200, 3000);
            // game loop
            delay(STARTSEQUENCEDELAY); 
            
            lightpattern();
            delay(lightsOutDelay);
            // LIGHTS OUT!
            pattern = 0;
            digitalWrite(SRLATCHPIN, LOW);                    // prepare to shift out from register
            shiftOut(SRDATAPIN, SRCLKPIN, MSBFIRST, pattern); // turn off lights
            digitalWrite(SRLATCHPIN, HIGH);                   // latch data to output
            
            lightsOutClock = millis();
            
            currentState = LIGHTSOUT;
            #ifdef DEBUG
            Serial.println("Lights out now!");
            #endif
            break;
        
        case LIGHTSOUT:
            if (jumpStartP1 || jumpStartP2) {
                // Jump start detected
                if (jumpStartP1 && !jumpStartP2) {
                    digitalWrite(P1JUMPLEDPIN, HIGH); // alert P1 jumped
                    #ifdef DEBUG
                        Serial.println("P1 jump start!");
                    #endif
                    lcd.backlight();
                    lcd.print("Jump start!");
                    lcd.setCursor(0,1);
                    lcd.print("P1 loses");
                } else if (jumpStartP2 && !jumpStartP1) {
                    digitalWrite(P2JUMPLEDPIN, HIGH); // alert P2 jumped
                    #ifdef DEBUG
                        Serial.println("P2 jump start!");
                    #endif
                    lcd.backlight();
                    lcd.print("Jump start!");
                    lcd.setCursor(0, 1);
                    lcd.print("P2 loses");
                } else {
                    digitalWrite(P1JUMPLEDPIN, HIGH); // alert both players  jumped at the same time
                    digitalWrite(P2JUMPLEDPIN, HIGH);
                    #ifdef DEBUG
                        Serial.println("simultaneous jump start!");
                    #endif
                    lcd.backlight();
                    lcd.print("Jump start!");
                    lcd.setCursor(0, 1);
                    lcd.print("Double fault!");
                }
                currentState = GAMEOVER;
            }
            else if (buttonPressedP1 || buttonPressedP2) {
                // Normal game
                if (buttonPressedP1 && buttonPressedP2) {
                    if (reactionTimeP1 < reactionTimeP2) {
                        winner = 1;
                        digitalWrite(P1LEDPIN, HIGH);
                    } else if (reactionTimeP2 < reactionTimeP1) {
                        winner = 2;
                        digitalWrite(P2LEDPIN, HIGH); 
                    } else {
                        winner = 3; // tie
                        digitalWrite(P1JUMPLEDPIN, HIGH);
                        digitalWrite(P2JUMPLEDPIN, HIGH);
                    }
                } else if (buttonPressedP1) {
                    winner = 1;
                    digitalWrite(P1LEDPIN, HIGH);
                } else if (buttonPressedP2) {
                    winner = 2;
                    digitalWrite(P2LEDPIN, HIGH);
                }
                currentState = GAMEOVER;
            }
            break;
        case GAMEOVER:
            // Display results and reset
            #ifdef DEBUG
                if (winner == 1) {
                //lcd function
                    Serial.print("Player 1 wins! Reaction time: ");
                    Serial.print(reactionTimeP1);
                    Serial.println(" ms");
                } else if (winner == 2) {

                    Serial.print("Player 2 wins! Reaction time: ");
                    Serial.print(reactionTimeP2);
                    Serial.println(" ms");

                } else if (winner == 3) {
                    Serial.println("Tie!");
                }
            #endif
            
            printWinMessage(winner);


            // Reset for next game
            winner = 0;

            delay(3000); // Short delay before next game
            currentState = LINEUP;
            buttonPressedP1 = false;
            buttonPressedP2 = false;
            jumpStartP1 = false;
            jumpStartP2 = false;
            break;
        }
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
void p1ButtonISR()
{
    if (currentState == STARTSEQUENCE) {
        jumpStartP1 = true;
    }
    else if (currentState == LIGHTSOUT && !buttonPressedP1) {         //game is running and handle debounce/holding buttons to prevent multiple 
        reactionTimeP1 = millis() - lightsOutClock;
        buttonPressedP1 = true;
    }
}

void p2ButtonISR() {
    if (currentState == STARTSEQUENCE) {
        jumpStartP2 = true;
    } else if (currentState == LIGHTSOUT && !buttonPressedP2) {       //game is running and handle debounce/holding buttons to prevent multiple 
        reactionTimeP2 = millis() - lightsOutClock;
        buttonPressedP2 = true;
    }
}

/*****
Purpose: Prints a message to the LCD to indicate who won the game

Parameter list:
  char winner - the winner as tracked by the game loop
Return value:
  void
*****/
void printWinMessage(char winner) {
    lcd.backlight();
    lcd.print("P");
    lcd.setCursor(1,0);
    lcd.print(winner);
    lcd.setCursor(4,0);
    lcd.print("Wins !Time:");
    lcd.setCursor(0, 1);
    if (winner == 1) {
        lcd.print(reactionTimeP1);
    } else {
        lcd.print(reactionTimeP2);  //in the case that it is a tie, p2's time will be printed (redundant fallback)
    }
    lcd.setCursor(5, 1);
    lcd.print("ms");
}
