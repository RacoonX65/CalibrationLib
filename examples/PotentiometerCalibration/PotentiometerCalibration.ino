/*
  PotentiometerCalibration.ino
  Example for CalibrationLib: Interactive Potentiometer Calibration

  This example demonstrates how to implement a complete calibration
  system for an analog potentiometer, with interactive user guidance
  and persistent storage of calibration values.

  Features:
  - Interactive calibration process
  - Multi-sample averaging for accuracy
  - Real-time value scaling
  - Persistent calibration storage
  - Default value fallback
  - Serial interface for control

  Calibration Process:
  1. User initiates calibration ('c' command)
  2. Set pot to minimum position
  3. System reads minimum value (10-sample average)
  4. Set pot to maximum position
  5. System reads maximum value (10-sample average)
  6. Values are saved to flash memory

  Calibration Parameters:
  - Minimum Value (raw ADC reading)
    * Default: 0
  - Maximum Value (raw ADC reading)
    * Default: 4095
  - Output Scale: 0-100%

  Hardware Setup:
  - ESP32 development board
  - Potentiometer:
    * Wiper -> GPIO34 (ADC1_CH6)
    * High  -> 3.3V
    * Low   -> GND
  - Serial connection (115200 baud)

  Serial Commands:
  - 'c': Start calibration process
  - 'r': Reset to default values
  - 'p': Print current calibration

  Storage:
  - Namespace: "pot_cal"
  - Keys: "min_val", "max_val"

  Dependencies:
  - ESP32 Arduino Core
  - CalibrationLib

  Author: Judas Sithole (judassithole@duck.com)
  Created: 2025
  License: MIT
*/

#include <CalibrationLib.h>

CalibrationLib calibration;

// Pin definition
const int POT_PIN = 34;

// Calibration values
int minValue = 0;
int maxValue = 4095;
float outputScale = 100.0; // Scale to 0-100%

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Potentiometer Calibration Example");
  Serial.println("==============================");
  
  // Initialize ADC
  analogReadResolution(12); // 12-bit resolution (0-4095)
  
  // Initialize calibration
  if (!calibration.begin("pot_cal")) {
    Serial.println("Failed to initialize calibration!");
    return;
  }
  
  // Load existing calibration if available
  if (calibration.hasCalibrationValue("min_val") && 
      calibration.hasCalibrationValue("max_val")) {
    calibration.getCalibrationValue("min_val", minValue);
    calibration.getCalibrationValue("max_val", maxValue);
    Serial.println("Loaded existing calibration values");
  } else {
    Serial.println("No calibration found. Using defaults");
  }
  
  Serial.println("\nCommands:");
  Serial.println("'c' - Start calibration");
  Serial.println("'r' - Reset to defaults");
  Serial.println("'p' - Print current values");
}

void calibratePotentiometer() {
  Serial.println("\nStarting calibration...");
  Serial.println("1. Turn potentiometer to minimum position");
  Serial.println("2. Press ENTER when ready");
  
  while (Serial.read() != '\n') {
    delay(10);
  }
  
  // Read minimum value (average of 10 readings)
  minValue = 0;
  for (int i = 0; i < 10; i++) {
    minValue += analogRead(POT_PIN);
    delay(50);
  }
  minValue /= 10;
  
  Serial.println("3. Turn potentiometer to maximum position");
  Serial.println("4. Press ENTER when ready");
  
  while (Serial.read() != '\n') {
    delay(10);
  }
  
  // Read maximum value (average of 10 readings)
  maxValue = 0;
  for (int i = 0; i < 10; i++) {
    maxValue += analogRead(POT_PIN);
    delay(50);
  }
  maxValue /= 10;
  
  // Save calibration
  calibration.setCalibrationValue("min_val", minValue);
  calibration.setCalibrationValue("max_val", maxValue);
  
  Serial.println("Calibration complete!");
  printValues();
}

void resetCalibration() {
  minValue = 0;
  maxValue = 4095;
  calibration.setCalibrationValue("min_val", minValue);
  calibration.setCalibrationValue("max_val", maxValue);
  Serial.println("Reset to default values");
  printValues();
}

void printValues() {
  Serial.println("\nCurrent calibration:");
  Serial.print("Minimum value: ");
  Serial.println(minValue);
  Serial.print("Maximum value: ");
  Serial.println(maxValue);
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    switch (cmd) {
      case 'c':
        calibratePotentiometer();
        break;
      case 'r':
        resetCalibration();
        break;
      case 'p':
        printValues();
        break;
    }
  }
  
  // Read and scale the potentiometer value
  int rawValue = analogRead(POT_PIN);
  float scaledValue = map(rawValue, minValue, maxValue, 0, outputScale);
  scaledValue = constrain(scaledValue, 0, outputScale);
  
  // Print values every second
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 1000) {
    Serial.print("Raw: ");
    Serial.print(rawValue);
    Serial.print("\tScaled: ");
    Serial.print(scaledValue, 1);
    Serial.println("%");
    lastPrint = millis();
  }
}