/*
  RGBLedCalibration.ino
  Example for CalibrationLib: RGB LED Color Balance Calibration

  This example demonstrates how to implement precise color calibration
  for RGB LEDs, allowing adjustment of individual color channels
  to achieve accurate color reproduction and white balance.

  Features:
  - Individual RGB channel calibration
  - Real-time color updates
  - White balance testing
  - Persistent calibration storage
  - Interactive serial interface
  - PWM-based LED control

  Calibration Parameters:
  1. Red Factor (0.0 - 1.0)
     - Adjusts red channel intensity
     - Default: 1.0

  2. Green Factor (0.0 - 1.0)
     - Adjusts green channel intensity
     - Default: 1.0

  3. Blue Factor (0.0 - 1.0)
     - Adjusts blue channel intensity
     - Default: 1.0

  Hardware Setup:
  - ESP32 development board
  - Common Cathode RGB LED:
    * Red   -> GPIO25 (via 220Ω resistor)
    * Green -> GPIO26 (via 220Ω resistor)
    * Blue  -> GPIO27 (via 220Ω resistor)
    * GND   -> GND

  PWM Configuration:
  - Frequency: 5000 Hz
  - Resolution: 8-bit (0-255)
  - Channels: R=0, G=1, B=2

  Serial Commands:
  - 'r': Adjust red factor
  - 'g': Adjust green factor
  - 'b': Adjust blue factor
  - 'w': Run white balance test
  - 's': Save calibration
  - 'p': Print current values

  Storage:
  - Namespace: "rgb_cal"
  - Keys: "red_factor", "green_factor", "blue_factor"

  Note: Resistor values may need adjustment based on your
  specific LED specifications. Typical values range from
  220Ω to 470Ω.

  Dependencies:
  - ESP32 Arduino Core
  - CalibrationLib

  Author: Judas Sithole (judassithole@duck.com)
  Created: 2025
  License: MIT
*/

#include <CalibrationLib.h>

CalibrationLib calibration;

// Pin definitions
const int RED_PIN = 25;
const int GREEN_PIN = 26;
const int BLUE_PIN = 27;

// PWM configuration
const int PWM_CHANNEL_R = 0;
const int PWM_CHANNEL_G = 1;
const int PWM_CHANNEL_B = 2;
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8; // 8-bit resolution (0-255)

// Calibration factors (0.0 - 1.0)
float redFactor = 1.0;
float greenFactor = 1.0;
float blueFactor = 1.0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("RGB LED Calibration Example");
  Serial.println("==========================");
  
  // Configure LED PWM channels
  ledcSetup(PWM_CHANNEL_R, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_G, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_B, PWM_FREQ, PWM_RESOLUTION);
  
  ledcAttachPin(RED_PIN, PWM_CHANNEL_R);
  ledcAttachPin(GREEN_PIN, PWM_CHANNEL_G);
  ledcAttachPin(BLUE_PIN, PWM_CHANNEL_B);
  
  // Initialize calibration
  if (!calibration.begin("rgb_cal")) {
    Serial.println("Failed to initialize calibration!");
    return;
  }
  
  // Load existing calibration if available
  if (calibration.hasCalibrationValue("red_factor") && 
      calibration.hasCalibrationValue("green_factor") &&
      calibration.hasCalibrationValue("blue_factor")) {
    calibration.getCalibrationValue("red_factor", redFactor);
    calibration.getCalibrationValue("green_factor", greenFactor);
    calibration.getCalibrationValue("blue_factor", blueFactor);
    Serial.println("Loaded existing calibration values");
  } else {
    Serial.println("No calibration found. Using defaults");
  }
  
  Serial.println("\nCommands:");
  Serial.println("'r' - Adjust red factor");
  Serial.println("'g' - Adjust green factor");
  Serial.println("'b' - Adjust blue factor");
  Serial.println("'w' - Test white balance");
  Serial.println("'s' - Save current calibration");
  Serial.println("'p' - Print current values");
}

void adjustColor(char color) {
  float* factor;
  const char* name;
  
  switch (color) {
    case 'r':
      factor = &redFactor;
      name = "Red";
      break;
    case 'g':
      factor = &greenFactor;
      name = "Green";
      break;
    case 'b':
      factor = &blueFactor;
      name = "Blue";
      break;
    default:
      return;
  }
  
  Serial.printf("\nAdjusting %s factor (current: %.2f)\n", name, *factor);
  Serial.println("Enter new factor (0.0 - 1.0):");
  
  while (!Serial.available()) {
    delay(10);
  }
  
  String input = Serial.readStringUntil('\n');
  float newFactor = input.toFloat();
  
  if (newFactor >= 0.0 && newFactor <= 1.0) {
    *factor = newFactor;
    Serial.printf("%s factor set to %.2f\n", name, *factor);
  } else {
    Serial.println("Invalid value! Factor must be between 0.0 and 1.0");
  }
}

void saveCalibration() {
  calibration.setCalibrationValue("red_factor", redFactor);
  calibration.setCalibrationValue("green_factor", greenFactor);
  calibration.setCalibrationValue("blue_factor", blueFactor);
  Serial.println("Calibration saved!");
}

void printValues() {
  Serial.println("\nCurrent calibration factors:");
  Serial.printf("Red: %.2f\n", redFactor);
  Serial.printf("Green: %.2f\n", greenFactor);
  Serial.printf("Blue: %.2f\n", blueFactor);
}

void setColor(int r, int g, int b) {
  // Apply calibration factors
  ledcWrite(PWM_CHANNEL_R, r * redFactor);
  ledcWrite(PWM_CHANNEL_G, g * greenFactor);
  ledcWrite(PWM_CHANNEL_B, b * blueFactor);
}

void testWhiteBalance() {
  Serial.println("Testing white balance...");
  
  // Cycle through colors
  Serial.println("Red");
  setColor(255, 0, 0);
  delay(1000);
  
  Serial.println("Green");
  setColor(0, 255, 0);
  delay(1000);
  
  Serial.println("Blue");
  setColor(0, 0, 255);
  delay(1000);
  
  Serial.println("White");
  setColor(255, 255, 255);
  delay(1000);
  
  // Turn off
  setColor(0, 0, 0);
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    switch (cmd) {
      case 'r':
      case 'g':
      case 'b':
        adjustColor(cmd);
        break;
      case 'w':
        testWhiteBalance();
        break;
      case 's':
        saveCalibration();
        break;
      case 'p':
        printValues();
        break;
    }
  }
}