//
// Created by Timo Stülten on 11.04.24.
//
#ifndef utils_h
#define utils_h

#include <Arduino.h>
#include <WString.h>

#define LED_BUILTIN 2

// ------------------------------------------------------------------------

// blink build-in LED
void blink() {
    const unsigned long BLINK_PHASE = 250000;
    // when to update LED
    static unsigned long nextChangeMicros = 0;
    static bool blink = false;

    unsigned long nowMicros = micros();
    if (nextChangeMicros < nowMicros) {
        nextChangeMicros = nowMicros + BLINK_PHASE;

        // Toggle LED
        digitalWrite(LED_BUILTIN, blink ? HIGH : LOW);
        blink = !blink;
    }
}

// ------------------------------------------------------------------------

void error(const String &s) {
    Serial.print("ERROR: ");
    Serial.println(s);
    bool blink = true;
    while (true) {
        digitalWrite(LED_BUILTIN, blink ? HIGH : LOW);
        blink = !blink;
        delay(250);
    }
}

#endif