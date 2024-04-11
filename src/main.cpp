/**
 *
 */
#include <Arduino.h>

#include "utils.cpp"
#include "colorsensor.cpp"
#include "servo.cpp"
#include "statemachine.cpp"


// simply calls how often loop() was called
unsigned long m_loopCount = 0;

// ------------------------------------------------------------------------

// Called on startup
void setup() {
    // Use contemporary baudrate and wait to get connection
    Serial.begin(921600);
    delay(100);
    Serial.println("Setup...");

    // Setup build-in LED
    pinMode(LED_BUILTIN, OUTPUT);
    blink();

    colorSetup();
    servoSetup();
    statemachineSetup();

    setNextEvent(Event::INITIALIZED);
}

void loop() {
    blink();
    m_loopCount++;

    Event event = checkForEvent();
    handleEvent(event);
}
