/* Lights Out - A 2 player reaction test game */
#include <Arduino.h>
#include <LCD_I2C.h>
//#include <Wire.h>

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
#define BUZZERTONEFREQ 400  //Hz for buzzer tone - play around until it sounds good
/*--------------GAME STATUS---------------*/
volatile bool gameRunning = false;
volatile bool buttonPressedP1 = false;
volatile bool buttonPressedP2 = false;
volatile bool buttonPressedBoth = false;
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
void blinkWinnerLED(int ledPin);

    /*=========================MAIN ARDUINO CODE===========================================================*/
    void setup() {
    pinMode(P1BUTTONPIN, INPUT); // CHANGED - TRY USING 10K resistor on board for cleaner read
    pinMode(P2BUTTONPIN, INPUT);
    #ifdef DEBUG
    Serial.begin(9600);
    #endif

    
    pinMode(P1LEDPIN, OUTPUT);
    pinMode(P2LEDPIN,OUTPUT);
    pinMode(P1JUMPLEDPIN,OUTPUT);
    pinMode(P2JUMPLEDPIN, OUTPUT);
    pinMode(BUZZERPIN, OUTPUT);
    pinMode(SRLATCHPIN, OUTPUT);
    pinMode(SRCLKPIN, OUTPUT);
    pinMode(SRDATAPIN,OUTPUT);

    
    lcd.begin(); //initialise adn turn on LCD
    lcd.backlight();
    lcd.print("Lights Out!");
    lcd.setCursor(0,1);
    lcd.print("A reaction game");
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Press both to");
    lcd.setCursor(0, 1);
    lcd.print("start...");

    #ifdef DEBUG
    Serial.print("Setup complete");
    Serial.print(buttonPressedP1);
    #endif
    

    attachInterrupt(digitalPinToInterrupt(P1BUTTONPIN), p1ButtonISR, RISING);      //ISR attached to button presses to allow game to keep running in loop and for accurate time detection
    attachInterrupt(digitalPinToInterrupt(P2BUTTONPIN), p2ButtonISR, RISING);      //CHANGED - inverted logic here for pull down resistors 
}

void loop() {
    switch (currentState) {
        case LINEUP:
            if (buttonPressedBoth) {      //game starts when both players press their buttons
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
            // game loop        
            lightsOutDelay = random(200, 3000);
            delay(STARTSEQUENCEDELAY); 
            lightpattern(); // 1,2,3,4,5
            delay(lightsOutDelay); 
            // LIGHTS OUT!
            pattern = 0;
            digitalWrite(SRLATCHPIN, LOW);                    // prepare to shift out from register
            shiftOut(SRDATAPIN, SRCLKPIN, MSBFIRST, pattern); // turn off lights
            digitalWrite(SRLATCHPIN, HIGH);                   // latch data to output
            
            lightsOutClock = millis();           
            currentState = LIGHTSOUT;

#ifdef DEBUG
            lcd.println("Lights out now!");
            #endif
            break;
        
        case LIGHTSOUT:
            if (jumpStartP1 || jumpStartP2) {
                // Jump start detected
                if (jumpStartP1 && !jumpStartP2) {
                    blinkWinnerLED(P1JUMPLEDPIN); // alert P1 jumped
                    #ifdef DEBUG
                        lcd.println("P1 jump start!");
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
            // Display results and reset
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

            delay(5000); // Short delay before next game
            buttonPressedP1 = false;
            buttonPressedP2 = false;
            buttonPressedBoth = false;
            jumpStartP1 = false;
            jumpStartP2 = false;
            lcd.clear();
            lcd.backlight();
            lcd.setCursor(0, 0);
            lcd.print("Hold both button");
            lcd.setCursor(0, 1);
            lcd.print("to start!");
            currentState = LINEUP;

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
    pattern = 0;
    for (int i = 0; i < 5; i++) {
      pattern = pattern | (1 << i);             // lights up each light in sequence by ORing each bit with the previous one shifted per second
  #ifdef DEBUG
      Serial.println(pattern, BIN);
  #endif
      digitalWrite(SRLATCHPIN, LOW);            // prepare to shift out from register
      shiftOut(SRDATAPIN, SRCLKPIN, MSBFIRST, pattern);
      digitalWrite(SRLATCHPIN, HIGH);           // latch data to output
      tone(BUZZERPIN, BUZZERTONEFREQ, 100);     //sound the buzzer at specified frequency for 100ms (tested this and this sounds good for lenght, play wiht                                     freq when in housing)
      delay(1000);                             // 1 second delay betwwen lightup as per f1 standard
    }
}
/*****
Purpose: ISR for p1 button
*****/
void p1ButtonISR() {
    #ifdef DEBUG
    Serial.println("P1 PRESSED");
    if (currentState == LINEUP) {
    #endif
    #ifndef DEBUG
    if (currentState == LINEUP && digitalRead(P2BUTTONPIN) == HIGH) {
    #endif
        buttonPressedBoth = true;
    } else if (currentState == STARTSEQUENCE) {
        jumpStartP1 = true;
    } else if (currentState == LIGHTSOUT && !buttonPressedP1) {         //game is running and handle debounce/holding buttons to prevent multiple 
        reactionTimeP1 = millis() - lightsOutClock;
        buttonPressedP1 = true;
    }
}

void p2ButtonISR() {
    #ifdef DEBUG
        Serial.println("P2 PRESSED");
    #endif
    if (currentState == LINEUP && digitalRead(P1BUTTONPIN) == HIGH) {
        buttonPressedBoth = true;
    } else if (currentState == STARTSEQUENCE) {
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
Return value:`
  void
*****/
void printWinMessage(char winner) {
    lcd.backlight();
    lcd.clear();
    lcd.print("P");
    lcd.setCursor(1,0);
    lcd.print((char)(winner + '0')); //prints the player by moving the ascii char to '0' (0x30)
    lcd.setCursor(3,0);
    lcd.print("Wins! Time:");
    lcd.setCursor(0, 1);
    if (winner == 1) {
        lcd.print(reactionTimeP1);
    } else {
        lcd.print(reactionTimeP2);  //in the case that it is a tie, p2's time will be printed (redundant fallback)
    }
    lcd.setCursor(4, 1);
    lcd.print("ms");
}

/*****
Purpose: Blink an led for the winner and then keep it on

Parameter list:
  int LEDPIN - pin of hte LED to flash

Return value:
  void
*****/
void blinkWinnerLED(int ledPin) {
    for (int i = 0; i < 5; i++) {
        digitalWrite(ledPin,HIGH);
        delay(100);
        digitalWrite(ledPin,LOW);
        delay(100);
    }
    digitalWrite(ledPin, HIGH);
}
