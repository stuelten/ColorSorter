//
// Created by Timo Stülten on 10.04.24.
//
#ifndef servo_h
#define servo_h

#include <Arduino.h>
#include <ESP32Servo.h>

// Servo PINs
#define SERVO_SEPARATOR_PIN 27
#define SERVO_LEVER_PIN 26

// Servo on the top
Servo servoSeparator;
Servo servoLever;

void positionLever(int pos) {
    servoLever.write(pos);
}

void positionSeparator(int pos) {
    servoSeparator.write(pos);
}

// ------------------------------------------------------------------------

void servoSetup() {
    // setup pins
    servoSeparator.attach(SERVO_SEPARATOR_PIN);
    servoLever.attach(SERVO_LEVER_PIN);
}

#endif
