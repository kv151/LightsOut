/* Lights Out - A 2 player reaction test game */
//#include <Arduino.h>
#include <LiquidCrystal_I2C.h>  // Official library
#include <Wire.h>

//#define DEBUG

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
#define BUZZERPIN 11
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

// Initialize LCD (address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

//put function declarations here:
void lightpattern();
void printWinMessage(char winner);
void p1ButtonISR();
void p2ButtonISR();
void blinkWinnerLED(int ledPin);

    /*=========================MAIN ARDUINO CODE===========================================================*/
    void setup()
{
    pinMode(P1BUTTONPIN, INPUT_PULLUP); // use an intenral pullup no resistor needed on the button
    pinMode(P2BUTTONPIN, INPUT_PULLUP);
    #ifdef DEBUG
    Serial.begin(9600);
    #endif

    pinMode(P1LEDPIN, OUTPUT);
    pinMode(P2LEDPIN,OUTPUT);
    pinMode(P1JUMPLEDPIN,OUTPUT);
    pinMode(P2JUMPLEDPIN, OUTPUT);
    pinMode(BUZZERPIN, OUTPUT);

    lcd.init();          // Initialize the LCD
    lcd.backlight();     // Turn on the backlight
    lcd.print("Lights Out!");
    lcd.setCursor(0,1);
    lcd.print("A reaction game");
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Press both to");
    lcd.setCursor(0, 1);
    lcd.print("start...");

    attachInterrupt(digitalPinToInterrupt(P1BUTTONPIN), p1ButtonISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(P2BUTTONPIN), p2ButtonISR, FALLING);
}

void loop() {
    switch (currentState) {
        case LINEUP:
            if (digitalRead(P1BUTTONPIN) == LOW) {
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Grid set");
                delay(1000);
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
            delay(STARTSEQUENCEDELAY);

            lightpattern();
            delay(lightsOutDelay);
            // LIGHTS OUT!
            pattern = 0;
            digitalWrite(SRLATCHPIN, LOW);
            shiftOut(SRDATAPIN, SRCLKPIN, MSBFIRST, pattern);
            digitalWrite(SRLATCHPIN, HIGH);

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
                    blinkWinnerLED(P1JUMPLEDPIN); // alert P1 jumped
                    #ifdef DEBUG
                        Serial.println("P1 jump start!");
                    #endif
                    lcd.backlight();
                    lcd.clear();
                    lcd.print("Jump start!");
                    lcd.setCursor(0,1);
                    lcd.print("P1 loses");
                } else if (jumpStartP2 && !jumpStartP1) {
                    blinkWinnerLED(P2JUMPLEDPIN); // alert P2 jumped
                    #ifdef DEBUG
                        Serial.println("P2 jump start!");
                    #endif
                    lcd.backlight();
                    lcd.clear();
                    lcd.print("Jump start!");
                    lcd.setCursor(0, 1);
                    lcd.print("P2 loses");
                } else {
                    digitalWrite(P1JUMPLEDPIN, HIGH); // alert both players jumped at the same time
                    digitalWrite(P2JUMPLEDPIN, HIGH);
                    #ifdef DEBUG
                        Serial.println("simultaneous jump start!");
                    #endif
                    lcd.backlight();
                    lcd.clear();
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
                        blinkWinnerLED(P1LEDPIN);
                    } else if (reactionTimeP2 < reactionTimeP1) {
                        winner = 2;
                        blinkWinnerLED(P2LEDPIN);
                    } else {
                        winner = 3; // tie
                        digitalWrite(P1JUMPLEDPIN, HIGH);
                        digitalWrite(P2JUMPLEDPIN, HIGH);
                    }
                } else if (buttonPressedP1) {
                    winner = 1;
                    blinkWinnerLED(P1LEDPIN);
                } else if (buttonPressedP2) {
                    winner = 2;
                    blinkWinnerLED(P2LEDPIN);
                }
                currentState = GAMEOVER;
            }
            break;

        case GAMEOVER:
            #ifdef DEBUG
                if (winner == 1) {
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

            if (!jumpStartP1 && !jumpStartP2) {
                printWinMessage(winner);
            }

            // Reset for next game
            winner = 0;
            delay(3000);
            currentState = LINEUP;
            buttonPressedP1 = false;
            buttonPressedP2 = false;
            jumpStartP1 = false;
            jumpStartP2 = false;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Press both to");
            lcd.setCursor(0, 1);
            lcd.print("start...");
            break;
    }
}

// Light pattern function (unchanged)
void lightpattern() {
    for (int i = 0; i < 5; i++) {
        pattern = pattern | (1 << i);
        #ifdef DEBUG
            Serial.println(pattern, BIN);
        #endif
        digitalWrite(SRLATCHPIN, LOW);
        shiftOut(SRDATAPIN, SRCLKPIN, MSBFIRST, pattern);
        digitalWrite(SRLATCHPIN, HIGH);
      	tone(BUZZERPIN,400,100);
        delay(1000);
    }
}

// ISRs (unchanged)
void p1ButtonISR() {
    if (currentState == STARTSEQUENCE) {
        jumpStartP1 = true;
    }
    else if (currentState == LIGHTSOUT && !buttonPressedP1) {
        reactionTimeP1 = millis() - lightsOutClock;
        buttonPressedP1 = true;
    }
}

void p2ButtonISR() {
    if (currentState == STARTSEQUENCE) {
        jumpStartP2 = true;
    } else if (currentState == LIGHTSOUT && !buttonPressedP2) {
        reactionTimeP2 = millis() - lightsOutClock;
        buttonPressedP2 = true;
    }
}

// LCD printing function (updated for LiquidCrystal_I2C)
void printWinMessage(char winner) {
    lcd.backlight();
    lcd.clear();
    lcd.print("P");
    lcd.setCursor(1,0);
    lcd.print((char)(winner + '0')); // Prints the player number as a character
    lcd.setCursor(3,0);
    lcd.print("Wins !Time:");
    lcd.setCursor(0, 1);
    if (winner == 1) {
        lcd.print(reactionTimeP1);
    } else if (winner == 2) {
        lcd.print(reactionTimeP2);
    } else {
        lcd.print("Tie!");
    }
    lcd.setCursor(4, 1);
    lcd.print("ms");
}

// Blink LED function (unchanged)
void blinkWinnerLED(int ledPin) {
    for (int i = 0; i < 5; i++) {  // Fixed: Initialize i
        digitalWrite(ledPin, HIGH);
        delay(100);
        digitalWrite(ledPin, LOW);
        delay(100);
    }
    digitalWrite(ledPin, HIGH);
}