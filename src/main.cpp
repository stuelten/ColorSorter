/**
 *
 */
#include <Arduino.h>
#include <ESP32Servo.h>

#define LED_BUILTIN 2
// simply calls how often loop() was called
unsigned long loopCount = 0;

// ------------------------------------------------------------------------

// Color Sensor PINs
#define COLOR_S2 4
#define COLOR_S3 5
#define COLOR_OUT_1 18
#define COLOR_OUT_2 19
#define COLOR_OUT_ENABLE_1 22
#define COLOR_OUT_ENABLE_2 23

// How long to wait for input after setting S2 and S3
#define COLOR_SENSOR_READ_DELAY 100

// Color sensors gives us red, green, blue and intensity
struct RGBI {
    int red = 0, green = 0, blue = 0, intensity = 0;
};

// We use two sensors
#define COLOR_SENSORS 2

// Measured values for all color sensors
struct ColorMeasurements {
    int val[COLOR_SENSORS];
};

// Global array to hold RGBI values for all sensors
static RGBI rgbArray[COLOR_SENSORS];

// ------------------------------------------------------------------------

// Servo PINs
#define SERVO_SEPERATOR_PIN 27
#define SERVO_LEVER_PIN 26

// Servo on the top
Servo servoSeparator;
Servo servoLever;

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

ColorMeasurements colorMeasure(int s2, int s3) {
    ColorMeasurements measurements{};

    // Set S2 and S3 pins according to the color to measure
    digitalWrite(COLOR_S2, s2);
    digitalWrite(COLOR_S3, s3);

    digitalWrite(COLOR_OUT_ENABLE_1, LOW);
    digitalWrite(COLOR_OUT_ENABLE_2, HIGH);
    delayMicroseconds(COLOR_SENSOR_READ_DELAY); // Short delay for stabilization
    measurements.val[0] = pulseIn(COLOR_OUT_1, LOW); // Measure from first sensor

    digitalWrite(COLOR_OUT_ENABLE_1, HIGH);
    digitalWrite(COLOR_OUT_ENABLE_2, LOW);
    delayMicroseconds(COLOR_SENSOR_READ_DELAY); // Short delay for stabilization
    measurements.val[1] = pulseIn(COLOR_OUT_2, LOW); // Measure from second sensor

    return measurements;
}

void fillRGBIArray() {
    ColorMeasurements red{}, green{}, blue{}, intensity{};

    red = colorMeasure(LOW, LOW);
    green = colorMeasure(HIGH, HIGH);
    blue = colorMeasure(LOW, HIGH);
    intensity = colorMeasure(HIGH, LOW);

    for (int sensorIndex = 0; sensorIndex < COLOR_SENSORS; ++sensorIndex) {
        rgbArray[sensorIndex].red = red.val[sensorIndex];
        rgbArray[sensorIndex].green = green.val[sensorIndex];
        rgbArray[sensorIndex].blue = blue.val[sensorIndex];
        rgbArray[sensorIndex].intensity = intensity.val[sensorIndex];
    }
}

// Accessor function to get the rgbArray from outside if needed.
RGBI *getRGBIArray() {
    fillRGBIArray(); // Ensure the array is filled with the latest values
    return rgbArray;
}

boolean checkForNewColorMeasurement() {
    const unsigned long MEASURE_PHASE = 200;
    // when to next read sensors
    static unsigned long nextChangeMicros = 0;

    unsigned long nowMicros = micros();
    if (nextChangeMicros < nowMicros) {
        nextChangeMicros = nowMicros + MEASURE_PHASE;
        return true;
    }
    return false;
}

void readColors() {
    static char buffer[80]; // Adjust the size based on the expected length
    static bool printEnabled = true; // Variable to control the printing
    static char color = ' ';

    // Check if any serial data is available to read
    if (Serial.available() > 0) {
        char receivedChar = Serial.read(); // Read the incoming byte
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
            Serial.print(loopCount);
            Serial.print(",");
            Serial.print(color);
            for (int sensorIndex = 0; sensorIndex < COLOR_SENSORS; ++sensorIndex) {
                snprintf(buffer, sizeof(buffer), ",%d,%03d,%03d,%03d,%03d",
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

void step1GetObject() {
    servoSeparator.write(0);
    delay(1000);
}

RGBI step2DetermineRGBI() {
    constexpr int ROUNDS = 5;

    RGBI rgbiTotal{};

    for (int i = 0; i < ROUNDS; ++i) {
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

    rgbiTotal.red = rgbiTotal.red / ROUNDS;
    rgbiTotal.green = rgbiTotal.green / ROUNDS;
    rgbiTotal.blue = rgbiTotal.blue / ROUNDS;
    rgbiTotal.intensity = rgbiTotal.intensity / ROUNDS;

    return rgbiTotal;
}

int step3CalculacteLeverPos(RGBI rgbi) {
    int pos = 90;

    return pos;
}

void step4PositionLever(int pos) {
    servoLever.write(pos);
}

void step5WaitForFinish() {
    delay(2000);
}

void process() {
    step1GetObject();
    RGBI rgbi = step2DetermineRGBI();
    int pos = step3CalculacteLeverPos(rgbi);
    step4PositionLever(pos);
    step5WaitForFinish();
}

// ------------------------------------------------------------------------

// Called on startup
void setup() {
    // Use contemporary baudrate
    Serial.begin(921600);
    delay(500);
    Serial.println("Setup...");

    // Setup build-in LED
    pinMode(LED_BUILTIN, OUTPUT);

    // Prepare color sensors
    pinMode(COLOR_S2, OUTPUT);
    pinMode(COLOR_S3, OUTPUT);
    pinMode(COLOR_OUT_1, INPUT);
    pinMode(COLOR_OUT_2, INPUT);

    // Servo
    servoSeparator.attach(SERVO_SEPERATOR_PIN);
    servoLever.attach(SERVO_LEVER_PIN);
}

// Called regularly
void loop() {
    blink();
    loopCount++;

    //readColors();
    //servoLoop();

    process();
}
