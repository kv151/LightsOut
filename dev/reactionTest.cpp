/* Fucction to test reaction timers and feedback  */
#include <Arduino.h>
// Define global definitions here:
#define LEDPIN 8 // pin for led
#define BUTTONPIN 2

// put function declarations here:
long int gameDelay = 0;
float reactionTime;
int buttonState = HIGH; //button active low

void setup()
{
    Serial.begin(9600);
    pinMode(LEDPIN, OUTPUT);
    pinMode(BUTTONPIN, INPUT_PULLUP);
}

void loop()
{
    buttonState = digitalRead(BUTTONPIN);
    
    if (buttonState == LOW) // button pushed
    { // button pressed
        Serial.println("Get ready!");
        Serial.println("-----------");
        for (int i = 0; i < 5; i++)
        {
            delay(1000);
            Serial.println(5 - i); // countdown
        }

        gameDelay = random(3000);
        delay(gameDelay);
        Serial.println("Light's out!");
    }

    reactionTime = millis();
    digitalWrite(LEDPIN, HIGH);
    buttonState = digitalRead(BUTTONPIN);

    if (buttonState == LOW) // button pushed
    { // button pulled
        digitalWrite(LEDPIN, LOW);
        reactionTime = millis() - reactionTime; // account for the delay to get to this  point in execution
        Serial.println("-----------");
        Serial.println("Your time was: ");
        Serial.print(reactionTime);
        Serial.println("ms");
        delay(2000);
        Serial.println("-----------");
        Serial.println("Push to go again");
    }
}

// put function definitions here: