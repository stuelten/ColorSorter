//
// Created by Timo Stülten on 10.04.24.
//
#ifndef colorsensor_h
#define colorsensor_h

#include <Arduino.h>

// PINs to set color to sense (R, G, B, Intensity), see below
#define COLOR_SELECT_2 4
#define COLOR_SELECT_3 5
// pin combinations for color to sense
#define COLOR_SELECT_RED LOW, LOW
#define COLOR_SELECT_GREEN HIGH, HIGH
#define COLOR_SELECT_BLUE LOW, HIGH
#define COLOR_SELECT_INTENSITY HIGH, LOW

// Out pin delivers sensor value
#define COLOR_OUT_1 18
#define COLOR_OUT_2 19
// Enable or disable sensor
// We must not enable both sensors at the same time
#define COLOR_OUT_ENABLE_1 22
#define COLOR_OUT_ENABLE_2 23

// How long to wait for input after setting S2 and S3 in micros
#define COLOR_SENSOR_READ_DELAY_MICROS 10
// Read multiple times and take the average
#define COLOR_SENSOR_READ_ROUNDS 5
// Only read sensor once per MEASURE_PHASE (in micros)
#define COLOR_SENSOR_MEASURE_PHASE_MICROS 200

// Color sensors give us red, green, blue and intensity
struct RGBI {
    unsigned long red = 0, green = 0, blue = 0, intensity = 0;
};

// We use two sensors
#define COLOR_SENSORS 2

// Measured values for all color sensors
struct ColorMeasurements {
    unsigned long values[COLOR_SENSORS];
};

// Global array to hold RGBI values for all sensors
static RGBI rgbArray[COLOR_SENSORS];

static RGBI currentColor{};

// How often did we read the sensor?
static unsigned long sensorCount = 0;

// ------------------------------------------------------------------------

// Do the low-level sensor reading on all sensors
ColorMeasurements colorMeasure(int sense2, int sense3) {
    ColorMeasurements measurements{};

    // Set S2 and S3 pins according to the color to measure
    digitalWrite(COLOR_SELECT_2, sense2);
    digitalWrite(COLOR_SELECT_3, sense3);

    digitalWrite(COLOR_OUT_ENABLE_1, LOW);
    digitalWrite(COLOR_OUT_ENABLE_2, HIGH);
    delayMicroseconds(COLOR_SENSOR_READ_DELAY_MICROS); // Short delay for stabilization
    measurements.values[0] = pulseIn(COLOR_OUT_1, LOW); // Measure from first sensor

    digitalWrite(COLOR_OUT_ENABLE_1, HIGH);
    digitalWrite(COLOR_OUT_ENABLE_2, LOW);
    delayMicroseconds(COLOR_SENSOR_READ_DELAY_MICROS); // Short delay for stabilization
    measurements.values[1] = pulseIn(COLOR_OUT_2, LOW); // Measure from second sensor

    return measurements;
}

// read all sensors and fill the rgbArray
void fillRGBIArray() {
    ColorMeasurements red{}, green{}, blue{}, intensity{};

    red = colorMeasure(COLOR_SELECT_RED);
    green = colorMeasure(COLOR_SELECT_GREEN);
    blue = colorMeasure(COLOR_SELECT_BLUE);
    intensity = colorMeasure(COLOR_SELECT_INTENSITY);

    for (int sensorIndex = 0; sensorIndex < COLOR_SENSORS; ++sensorIndex) {
        rgbArray[sensorIndex].red = red.values[sensorIndex];
        rgbArray[sensorIndex].green = green.values[sensorIndex];
        rgbArray[sensorIndex].blue = blue.values[sensorIndex];
        rgbArray[sensorIndex].intensity = intensity.values[sensorIndex];
    }
}

// Accessor function to get the rgbArray from outside if needed.
RGBI *getRGBIArray() {
    fillRGBIArray(); // Ensure the array is filled with the latest values
    return rgbArray;
}

// only check for new color once every MEASURE_PHASE micros
boolean checkForNewColorMeasurement() {
    // when to next read sensors
    static unsigned long nextChangeMicros = 0;

    unsigned long nowMicros = micros();
    if (nextChangeMicros < nowMicros) {
        nextChangeMicros = nowMicros + COLOR_SENSOR_MEASURE_PHASE_MICROS;
        return true;
    }
    return false;
}

// ------------------------------------------------------------------------

RGBI readRGBI() {
    RGBI rgbiTotal{};

    for (int i = 0; i < COLOR_SENSOR_READ_ROUNDS; ++i) {
        RGBI rgbi{};
        fillRGBIArray();
        for (int sensorIndex = 0; sensorIndex < COLOR_SENSORS; ++sensorIndex) {
            rgbi.red += rgbArray[sensorIndex].red;
            rgbi.green += rgbArray[sensorIndex].green;
            rgbi.blue += rgbArray[sensorIndex].blue;
            rgbi.intensity += rgbArray[sensorIndex].intensity;
        }
        rgbi.red = rgbi.red / COLOR_SENSORS;
        rgbi.green = rgbi.green / COLOR_SENSORS;
        rgbi.blue = rgbi.blue / COLOR_SENSORS;
        rgbi.intensity = rgbi.intensity / COLOR_SENSORS;

        rgbiTotal.red += rgbi.red;
        rgbiTotal.green += rgbi.green;
        rgbiTotal.blue += rgbi.blue;
        rgbiTotal.intensity += rgbi.intensity;
    }

    rgbiTotal.red = rgbiTotal.red / COLOR_SENSOR_READ_ROUNDS;
    rgbiTotal.green = rgbiTotal.green / COLOR_SENSOR_READ_ROUNDS;
    rgbiTotal.blue = rgbiTotal.blue / COLOR_SENSOR_READ_ROUNDS;
    rgbiTotal.intensity = rgbiTotal.intensity / COLOR_SENSOR_READ_ROUNDS;

    currentColor = rgbiTotal;
    return rgbiTotal;
}

// ------------------------------------------------------------------------

void printColors() {
    static char buffer[80]; // Adjust the size based on the expected length
    static bool printEnabled = true; // Variable to control the printing
    static char color = ' ';

    // Check if any serial data is available to read
    if (Serial.available() > 0) {
        char receivedChar = (char) Serial.read(); // Read the incoming byte
        // Enable or disable printing based on the received command
        if (receivedChar == '1') {
            printEnabled = true;
        } else if (receivedChar == '0') {
            printEnabled = false;
        } else {
            color = receivedChar;
            Serial.println("Nr,Color,Sensor0,Red0,Green0,Blue0,Intensity0,Sensor1,Red1,Green1,Blue1,Intensity1");
        }
    }

    // Example usage: Print a message if printing is enabled
    if (checkForNewColorMeasurement()) {
        // Read color sensors
        fillRGBIArray();

        if (printEnabled) {
            // Print measurements for each sensor
            Serial.print(sensorCount++);
            Serial.print(",");
            Serial.print(color);
            for (int sensorIndex = 0; sensorIndex < COLOR_SENSORS; ++sensorIndex) {
                snprintf(buffer, sizeof(buffer), ",%d,%03lu,%03lu,%03lu,%03lu",
                         sensorIndex,
                         rgbArray[sensorIndex].red, rgbArray[sensorIndex].green, rgbArray[sensorIndex].blue,
                         rgbArray[sensorIndex].intensity
                );
                Serial.print(buffer);
            }
            Serial.println();
        }
    }
}

// ------------------------------------------------------------------------

void colorSetup() {
    // setup selector pins
    pinMode(COLOR_SELECT_2, OUTPUT);
    pinMode(COLOR_SELECT_3, OUTPUT);

    // setup pins giving sensor value
    pinMode(COLOR_OUT_1, INPUT);
    pinMode(COLOR_OUT_2, INPUT);

    // setup pins which enable sensors
    pinMode(COLOR_OUT_ENABLE_1, OUTPUT);
    pinMode(COLOR_OUT_ENABLE_2, OUTPUT);
}

#endif
