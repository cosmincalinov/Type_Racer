#include <Arduino.h>

// Pin definitions
const int ledPinRed = 9;
const int ledPinGreen = 10;
const int ledPinBlue = 11;
const int buttonStartStop = 2;
const int buttonDifficulty = 3;

// Enum for the state of the game, dificulty and color of the RGBLed
enum GameState { IDLE, COUNTDOWN, RUNNING, FINISHED };
enum Difficulty { EASY, MEDIUM, HARD };

// Initial state of the game
GameState gameState = IDLE;
Difficulty difficulty = EASY;

// Time and variables for the reverse countdown
unsigned long countdownStartTime = 0;
unsigned long lastCountdownUpdateTime = 0;
int countdownSecondsLeft = 3;

unsigned long roundStartTime = 0;
unsigned long wordDisplayTime = 0;
int wordsCorrect = 0;

// Words
const char* words[] = {"robo", "cosmin", "adi", "raluca", "buton", "lab", "curs"};
const int numWords = 7;

// Dificulty settings
unsigned long difficultyTimes[] = {3000, 2000, 1000}; // Easy, Medium, Hard

// Debouncing
unsigned long lastDebounceTimeStartStop = 0;
unsigned long lastDebounceTimeDifficulty = 0;
const unsigned long debounceDelay = 50;  // 50 ms for debouncing

// button state flags
volatile bool startStopFlag = false;
volatile bool difficultyFlag = false;

// sets the color of the RGBLed
void setLEDColor(int col) {
    switch (col) {
      case (1): {
        // red
        digitalWrite(ledPinRed, HIGH);
        digitalWrite(ledPinGreen, LOW);
        digitalWrite(ledPinBlue, LOW);
        break;
      }
      case (2): {
        // green
        digitalWrite(ledPinRed, LOW);
        digitalWrite(ledPinGreen, HIGH);
        digitalWrite(ledPinBlue, LOW);
        break;
      }
      case (3): {
        // white
        digitalWrite(ledPinRed, HIGH);
        digitalWrite(ledPinGreen, HIGH);
        digitalWrite(ledPinBlue, HIGH);
        break;
      }
      default: {
        // off
        digitalWrite(ledPinRed, LOW);
        digitalWrite(ledPinGreen, LOW);
        digitalWrite(ledPinBlue, LOW);
        break;
      }
    }
}

// initiate countdown
void startCountdown() {
    countdownStartTime = millis();
    lastCountdownUpdateTime = millis();
    countdownSecondsLeft = 3;
    gameState = COUNTDOWN;
    Serial.println("Countdown starting...");
}
// updates countdown
void updateCountdown() {
    if (gameState == COUNTDOWN) {
        unsigned long currentMillis = millis();
        
        // if one second has passed since the last update
        if (currentMillis - lastCountdownUpdateTime >= 1000) {
            lastCountdownUpdateTime = currentMillis;
            Serial.println(countdownSecondsLeft);
            setLEDColor(3); // white LED

            countdownSecondsLeft--;

            // Start round if countdown has stopped
            if (countdownSecondsLeft <= 0) {
                gameState = RUNNING;
                roundStartTime = millis();
                wordDisplayTime = millis();
                setLEDColor(2); // green LED
                Serial.println("Go!");
            }
        }
    }
}

// change difficulty setting
void cycleDifficulty() {
    difficulty = (Difficulty)((difficulty + 1) % 3);
    const char* difficultyText[] = {"Easy mode on!", "Medium mode on!", "Hard mode on!"};
    Serial.println(difficultyText[difficulty]);
}

// initiate a round
void playRound() {
    if (millis() - roundStartTime >= 30000) { // 30 seconds for a round
        gameState = FINISHED;
    } else {
        const char* word = words[random(0, numWords)];
        
        // changes the speed of showing a new word if the difficulty is different
        if (millis() - wordDisplayTime >= difficultyTimes[difficulty]) {
            wordDisplayTime = millis();
            Serial.print("Type the word: ");
            Serial.println(word);
        }
        
        // if word is correct
        if (Serial.available() > 0) {
            String input = Serial.readStringUntil('\n');
            if (input.equals(word)) {
                wordsCorrect++;
                setLEDColor(2); // green LED
                wordDisplayTime = millis();
            } else {
                setLEDColor(1); // red LED
            }
        }
    }
}

// end the round and show the final score
void endRound() {
    Serial.print("Round over! Words typed correctly: ");
    Serial.println(wordsCorrect);
    setLEDColor(3); // white LED
    gameState = IDLE;
}

// ISR for Start/Stop button
void onStartStopPress() {
    startStopFlag = true;  // flag that indicates the button being pressed
}

// ISR for difficulty
void onDifficultyPress() {
    difficultyFlag = true;  // flag that indicates the button being pressed
}

void setup() {
    Serial.begin(9600);

    // RGB Led setup
    pinMode(ledPinRed, OUTPUT);
    pinMode(ledPinGreen, OUTPUT);
    pinMode(ledPinBlue, OUTPUT);
    setLEDColor(3); // white LED

    // button setup
    pinMode(buttonStartStop, INPUT_PULLUP);
    pinMode(buttonDifficulty, INPUT_PULLUP);

    // interrupts to buttons
    attachInterrupt(digitalPinToInterrupt(buttonStartStop), onStartStopPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(buttonDifficulty), onDifficultyPress, FALLING);
}

void loop() {
    unsigned long currentMillis = millis();

    // Debouncing for butonul Start/Stop
    if (startStopFlag && (currentMillis - lastDebounceTimeStartStop > debounceDelay)) {
        lastDebounceTimeStartStop = currentMillis;  // reset debounce time
        startStopFlag = false;  // reset flag
        
        if (gameState == IDLE) {
            startCountdown();
        } else if (gameState == RUNNING) {
            gameState = FINISHED;
        }
    }

    // Debouncing for difficulty button
    if (difficultyFlag && (currentMillis - lastDebounceTimeDifficulty > debounceDelay)) {
        lastDebounceTimeDifficulty = currentMillis;  // reset debounce time
        difficultyFlag = false;  // reset flag
        
        if (gameState == IDLE) {
            cycleDifficulty();
        }
    }

    // handle the state of the game
    switch (gameState) {
        case IDLE:
            setLEDColor(3); // white LED
            break;
        case COUNTDOWN:
            updateCountdown(); // update reverse countdown
            break;
        case RUNNING:
            playRound(); // plays the round
            break;
        case FINISHED:
            endRound(); // end the round and reset the game
            break;
    }
}
