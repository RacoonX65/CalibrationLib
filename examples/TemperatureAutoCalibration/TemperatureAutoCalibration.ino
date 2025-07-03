/*
  TemperatureAutoCalibration.ino
  Example for CalibrationLib: Automated Temperature Sensor Calibration

  This example demonstrates an automated two-point calibration process
  for temperature sensors using CalibrationLib. It implements a
  professional calibration workflow using ice water (0°C) and boiling
  water (100°C) as reference points.

  Features:
  - Two-point calibration system
  - Automatic parameter calculation
  - Persistent calibration storage
  - Averaged readings for accuracy
  - Linear correction application
  - Calibration status tracking
  - User-guided calibration process

  Calibration Process:
  1. Low Reference Point (0°C)
     - Uses ice water bath
     - Takes 10 averaged readings
  2. High Reference Point (100°C)
     - Uses boiling water bath
     - Takes 10 averaged readings
  3. Automatic Calculation
     - Computes scale factor
     - Determines offset value

  Calibration Parameters:
  - offset: Zero-point correction
  - scale: Multiplication factor
  - calibrated: Status flag

  Calibration Formula:
  temperature = (raw_reading - offset) * scale

  Hardware Setup:
  - ESP32 development board
  - Temperature sensor on GPIO36 (ADC1_CH0)
  - Serial connection for interaction (115200 baud)

  Required Materials:
  - Ice water bath (0°C reference)
  - Boiling water bath (100°C reference)
  - Container for water baths
  - Thermometer for verification

  Safety Notes:
  - Use appropriate safety measures when handling hot water
  - Ensure sensor is waterproof or properly protected
  - Verify water bath temperatures with a reference thermometer

  Serial Interface:
  - Press 'c' to start/continue calibration process
  - Continuous output of raw and calibrated readings

  Dependencies:
  - ESP32 Arduino Core
  - CalibrationLib

  Note: This example uses a simulated sensor response.
  For real applications, replace readRawTemperature()
  with actual sensor reading implementation.

  Author: Judas Sithole (judassithole@duck.com)
  Created: 2025
  License: MIT
*/

#include <CalibrationLib.h>

CalibrationLib calibration;

// Simulated temperature sensor pin
const int tempSensorPin = 36;

// Calibration parameters
float offset = 0.0;
float scale = 1.0;

// Reference temperatures for calibration (in Celsius)
const float REF_TEMP_LOW = 0.0;   // Ice water reference (0°C)
const float REF_TEMP_HIGH = 100.0; // Boiling water reference (100°C)

// Flag to track if calibration has been performed
bool isCalibrated = false;

void setup() {
  Serial.begin(115200);
  delay(1000); // Give time to open serial monitor
  
  Serial.println("\nTemperature Sensor Auto-Calibration Example");
  Serial.println("=========================================\n");
  
  // Initialize the calibration library
  if (!calibration.begin("tempsensor")) {
    Serial.println("Failed to initialize calibration library!");
    return;
  }
  
  // Check if calibration data exists
  if (calibration.hasCalibrationValue("offset") && 
      calibration.hasCalibrationValue("scale") &&
      calibration.hasCalibrationValue("calibrated")) {
    
    // Load existing calibration
    calibration.getCalibrationValue("offset", offset);
    calibration.getCalibrationValue("scale", scale);
    calibration.getCalibrationValue("calibrated", isCalibrated);
    
    if (isCalibrated) {
      Serial.println("Loaded existing calibration data:");
      Serial.print("Offset: ");
      Serial.print(offset, 4);
      Serial.print(" | Scale: ");
      Serial.println(scale, 6);
      Serial.println("\nReady to measure temperatures!");
    } else {
      Serial.println("Previous calibration was incomplete. Starting new calibration...");
      startCalibrationProcess();
    }
  } else {
    Serial.println("No calibration data found. Starting calibration process...");
    startCalibrationProcess();
  }
}

void loop() {
  if (isCalibrated) {
    // Read and display calibrated temperature
    float rawTemp = readRawTemperature();
    float calibratedTemp = (rawTemp - offset) * scale;
    
    Serial.print("Raw ADC: ");
    Serial.print(rawTemp);
    Serial.print(" | Temperature: ");
    Serial.print(calibratedTemp, 2);
    Serial.println(" °C");
    
    delay(1000);
  } else {
    // Check if user wants to start calibration
    if (Serial.available() && Serial.read() == 'c') {
      startCalibrationProcess();
    }
  }
}

void startCalibrationProcess() {
  Serial.println("\n--- Starting Auto-Calibration Process ---");
  Serial.println("This process requires two reference temperature points.");
  
  // Step 1: Low temperature reference point (ice water, 0°C)
  Serial.println("\nStep 1: Place the sensor in ice water (0°C).");
  Serial.println("Press 'c' when ready...");
  waitForKey('c');
  
  float lowReading = readAveragedRawTemperature(10);
  Serial.print("Low reference reading: ");
  Serial.println(lowReading);
  
  // Step 2: High temperature reference point (boiling water, 100°C)
  Serial.println("\nStep 2: Place the sensor in boiling water (100°C).");
  Serial.println("WARNING: Be careful with boiling water!");
  Serial.println("Press 'c' when ready...");
  waitForKey('c');
  
  float highReading = readAveragedRawTemperature(10);
  Serial.print("High reference reading: ");
  Serial.println(highReading);
  
  // Calculate calibration parameters
  // For a linear sensor: temp = (raw - offset) * scale
  scale = (REF_TEMP_HIGH - REF_TEMP_LOW) / (highReading - lowReading);
  offset = lowReading - (REF_TEMP_LOW / scale);
  
  // Save calibration data
  calibration.setCalibrationValue("offset", offset);
  calibration.setCalibrationValue("scale", scale);
  isCalibrated = true;
  calibration.setCalibrationValue("calibrated", isCalibrated);
  
  Serial.println("\nCalibration completed and saved!");
  Serial.print("Offset: ");
  Serial.print(offset, 4);
  Serial.print(" | Scale: ");
  Serial.println(scale, 6);
  Serial.println("\nReady to measure temperatures!");
}

float readRawTemperature() {
  // Read the analog value from the temperature sensor
  int adcValue = analogRead(tempSensorPin);
  
  // For this example, we'll simulate a temperature sensor
  // In a real application, you would convert the ADC value to a raw temperature
  // based on the sensor's datasheet
  
  // Simulate a sensor with non-linear response
  float rawTemp = (adcValue / 16.0) - 50.0; // Just a simulation
  
  return rawTemp;
}

float readAveragedRawTemperature(int samples) {
  float sum = 0.0;
  for (int i = 0; i < samples; i++) {
    sum += readRawTemperature();
    delay(100);
  }
  return sum / samples;
}

void waitForKey(char key) {
  while (Serial.available()) { Serial.read(); } // Clear buffer
  char input = 0;
  while (input != key) {
    if (Serial.available()) {
      input = Serial.read();
    }
    delay(10);
  }
}