#include <Arduino.h>

// Pin definitions
const int ledPinRed = 9;
const int ledPinGreen = 10;
const int ledPinBlue = 11;
const int buttonStartStop = 2;
const int buttonDifficulty = 3;
const char backspaceChar = 8; 

bool begin = 1;

// Enum for the state of the game, difficulty, and color of the RGB LED
enum GameState { IDLE, COUNTDOWN, RUNNING, FINISHED };
enum Difficulty { EASY, MEDIUM, HARD };
enum Colors { RED, GREEN, WHITE, OFF };

// Initial state of the game
GameState gameState = IDLE;
Difficulty difficulty = EASY;

// Time and variables for the countdown and round
unsigned long countdownStartTime = 0;
unsigned long lastCountdownUpdateTime = 0;
unsigned long lastBlinkTime = 0;  // Time tracker for LED blink during countdown
int countdownSecondsLeft = 3;

unsigned long roundStartTime = 0;
unsigned long wordDisplayTime = 0;
int wordsCorrect = 0;

// Words
String userInput = ""; // current input of the user
const char* currentWord = "";  // Inițializare ca string gol, accesibilă global
const char* words[] = {"robo", "cosmin", "adi", "raluca", "buton", "lab", "curs"};
const int numWords = 7;

// Difficulty settings
unsigned long difficultyTimes[] = {7000, 5000, 3000}; // Easy, Medium, Hard

// Debouncing
unsigned long lastDebounceTimeStartStop = 0;
unsigned long lastDebounceTimeDifficulty = 0;
const unsigned long debounceDelay = 50;  // 50 ms for debouncing

// Button state flags
volatile bool startStopFlag = false;
volatile bool difficultyFlag = false;

// Sets the color of the RGB LED
void setLEDColor(Colors col) {
    switch (col) {
      case RED:
        digitalWrite(ledPinRed, HIGH);
        digitalWrite(ledPinGreen, LOW);
        digitalWrite(ledPinBlue, LOW);
        break;
      case GREEN:
        digitalWrite(ledPinRed, LOW);
        digitalWrite(ledPinGreen, HIGH);
        digitalWrite(ledPinBlue, LOW);
        break;
      case WHITE:
        digitalWrite(ledPinRed, HIGH);
        digitalWrite(ledPinGreen, HIGH);
        digitalWrite(ledPinBlue, HIGH);
        break;
      default:
        digitalWrite(ledPinRed, LOW);
        digitalWrite(ledPinGreen, LOW);
        digitalWrite(ledPinBlue, LOW);
        break;
    }
}

// Initiate countdown
void startCountdown() {
    countdownStartTime = millis();
    lastCountdownUpdateTime = millis();
    lastBlinkTime = millis();  // Initialize blink time for countdown
    countdownSecondsLeft = 3;
    gameState = COUNTDOWN;
    Serial.println("Countdown starting...");
}

// Update countdown with LED blink
void updateCountdown() {
    if (gameState == COUNTDOWN) {
        unsigned long currentMillis = millis();

        // if we are at the start of a second
        if (countdownSecondsLeft > 0 && (currentMillis - lastCountdownUpdateTime >= 1000)) {
            lastCountdownUpdateTime = currentMillis;
            countdownSecondsLeft--;

            Serial.println(countdownSecondsLeft + 1);  // print the current number
            setLEDColor(WHITE);  // white LED
        }

        //  Stop LED after 500 ms
        if (currentMillis - lastCountdownUpdateTime >= 500 && countdownSecondsLeft > 0) {
            setLEDColor(OFF);  // LED off
        }

        // Start round after countdown reaches 0
        if (countdownSecondsLeft <= 0) {
            gameState = RUNNING;
            roundStartTime = millis();
            wordDisplayTime = millis();
            setLEDColor(GREEN);  // green LED
            Serial.println("Go!");
        }
    }
}

// Change difficulty setting
void cycleDifficulty() {
    difficulty = (Difficulty)((difficulty + 1) % 3);
    const char* difficultyText[] = {"Easy mode on!", "Medium mode on!", "Hard mode on!"};
    Serial.println(difficultyText[difficulty]);
}

// Play round
void playRound() {
    // verify if a round lasted more than 30 seconds
    if (millis() - roundStartTime >= 30000) {
        gameState = FINISHED;bool begin = 1;
    } else {
        // generate a random word and set its difficulty time
        if (begin || millis() - wordDisplayTime >= difficultyTimes[difficulty]) {
            wordDisplayTime = millis();
            currentWord = words[random(0, numWords)];
            Serial.println("Type the word: ");
            Serial.println(currentWord);

            // reset the input for the user
            userInput = "";

            begin = 0;  
        }

        // if there is an available input
        while (Serial.available() > 0) {
            char incomingChar = Serial.read();  // read a char

            if (incomingChar == backspaceChar) {
                // to find backspace - delete the last char
                if (userInput.length() > 0) {
                    userInput.remove(userInput.length() - 1);
                    Serial.print("\b \b");  // delete the char from serial monitor
                }
            } else if (incomingChar == '\n' || incomingChar == '\r') {
                // ignore enter and new line
                continue;
            } else {
                // add char in user input
                userInput += incomingChar;
                Serial.print(incomingChar);  // display char in serial monitor
            }

            // verify of the user input is correct
            if (userInput.equals(currentWord)) {
                wordsCorrect++;
                setLEDColor(GREEN);  // green LED
                wordDisplayTime = millis();  // reset time for the next word
                break;  // go to the new word
            } else if (!String(currentWord).startsWith(userInput)) {
                // conversion to string
                setLEDColor(RED);  // red LED
            }
        }
    }
}


// End the round and show the score
void endRound() {
    Serial.print("Round over! Words typed correctly: ");
    Serial.println(wordsCorrect);
    setLEDColor(WHITE); // white LED for idle mode
    gameState = IDLE;
}

// ISR for Start/Stop button
void onStartStopPress() {
    startStopFlag = true;  // Flag that indicates the button was pressed
}

// ISR for Difficulty button
void onDifficultyPress() {
    difficultyFlag = true;  // Flag that indicates the button was pressed
}

void setup() {
    Serial.begin(9600);

    // RGB LED setup
    pinMode(ledPinRed, OUTPUT);
    pinMode(ledPinGreen, OUTPUT);
    pinMode(ledPinBlue, OUTPUT);
    setLEDColor(WHITE); // White LED for idle mode

    // Button setup
    pinMode(buttonStartStop, INPUT_PULLUP);
    pinMode(buttonDifficulty, INPUT_PULLUP);

    // Attach interrupts to buttons
    attachInterrupt(digitalPinToInterrupt(buttonStartStop), onStartStopPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(buttonDifficulty), onDifficultyPress, FALLING);
}

void loop() {
    unsigned long currentMillis = millis();

    // Debouncing for Start/Stop button
    if (startStopFlag && (currentMillis - lastDebounceTimeStartStop > debounceDelay)) {
        lastDebounceTimeStartStop = currentMillis;  // Reset debounce time
        startStopFlag = false;  // Reset flag
        
        if (gameState == IDLE) {
            startCountdown();
        } else if (gameState == RUNNING) {
            gameState = FINISHED;
        }
    }

    // Debouncing for Difficulty button
    if (difficultyFlag && (currentMillis - lastDebounceTimeDifficulty > debounceDelay)) {
        lastDebounceTimeDifficulty = currentMillis;  // Reset debounce time
        difficultyFlag = false;  // Reset flag
        
        if (gameState == IDLE) {
            cycleDifficulty();
        }
    }

    // Handle game state
    switch (gameState) {
        case IDLE:
            setLEDColor(WHITE); // White LED for idle mode
            break;
        case COUNTDOWN:
            updateCountdown(); // Update countdown with LED blink
            break;
        case RUNNING:
            playRound(); // Play round
            break;
        case FINISHED:
            endRound(); // End round and reset game
            break;
    }
}
