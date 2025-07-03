/*
  SensorCalibrationWorkflow.ino
  Example for CalibrationLib: Interactive Sensor Calibration Process

  This example demonstrates a complete, user-guided workflow for sensor
  calibration using CalibrationLib. It implements a professional
  calibration process including:
  - Step-by-step guided calibration
  - Persistent storage of calibration data
  - Real-time sensor reading with calibration applied
  - Option to recalibrate existing sensors

  Features:
  - Interactive calibration wizard
  - Zero-point calibration
  - Scale factor calculation
  - Maximum value adjustment
  - Persistent calibration storage
  - Real-time calibrated readings
  - Calibration data verification
  - User-friendly serial interface

  Calibration Process:
  1. Zero Point Calibration
     - Set sensor to minimum position
     - Record baseline reading
  2. Maximum Value Calibration
     - Set sensor to maximum position
     - Record peak reading
  3. Scale Factor Configuration
     - Set desired maximum output value
     - Automatically calculate scale factor

  Calibration Parameters:
  - zero: Sensor reading at minimum position
  - scale: Multiplication factor for scaling
  - max: Desired maximum output value

  Hardware Setup:
  - ESP32 development board
  - Analog sensor connected to GPIO36 (ADC1_CH0)
  - Serial connection for interaction (115200 baud)

  Serial Interface Commands:
  - 'y': Start recalibration process
  - 'n': Skip recalibration
  - Any key: Proceed through calibration steps
  - Enter number: Set maximum output value

  Dependencies:
  - ESP32 Arduino Core
  - CalibrationLib

  Note: This example uses a simulated analog sensor.
  For real applications, replace analogRead() with
  your specific sensor reading implementation.

  Author: Judas Sithole (judassithole@duck.com)
  Created: 2025
  License: MIT
*/


#include <CalibrationLib.h>

CalibrationLib calibration;

// Calibration parameters
int zeroPoint = 0;
float scaleFactor = 1.0;
int maxValue = 1023;

// Simulated sensor pin
const int sensorPin = 36; // ADC pin on ESP32

void setup() {
  Serial.begin(115200);
  delay(1000); // Give time to open serial monitor
  
  Serial.println("\nSensor Calibration Workflow Example");
  Serial.println("==================================\n");
  
  // Initialize the calibration library
  if (!calibration.begin("sensor1")) {
    Serial.println("Failed to initialize calibration library!");
    return;
  }
  
  // Check if calibration data exists
  if (calibration.hasCalibrationValue("zero") && 
      calibration.hasCalibrationValue("scale") &&
      calibration.hasCalibrationValue("max")) {
    
    // Load existing calibration
    loadCalibrationData();
    
    Serial.println("Existing calibration loaded.");
    printCalibrationValues();
    
    // Ask if user wants to recalibrate
    Serial.println("\nDo you want to recalibrate? (y/n)");
    while (!Serial.available()) {
      // Wait for input
    }
    
    if (Serial.read() == 'y') {
      performCalibration();
    }
  } else {
    Serial.println("No calibration data found. Starting calibration process...");
    performCalibration();
  }
  
  Serial.println("\nCalibration complete. Now reading sensor values...");
  Serial.println("Press any key to end the program.");
}

void loop() {
  // Read and display calibrated sensor values
  int rawValue = analogRead(sensorPin);
  float calibratedValue = ((float)rawValue - zeroPoint) * scaleFactor;
  
  // Constrain to valid range
  if (calibratedValue < 0) calibratedValue = 0;
  if (calibratedValue > maxValue) calibratedValue = maxValue;
  
  Serial.print("Raw: ");
  Serial.print(rawValue);
  Serial.print("\tCalibrated: ");
  Serial.println(calibratedValue, 2);
  
  // Check if user wants to end
  if (Serial.available()) {
    calibration.end();
    Serial.println("\nProgram ended. Calibration data saved.");
    while(1); // Stop execution
  }
  
  delay(500); // Update every half second
}

void performCalibration() {
  Serial.println("\n--- Starting Calibration Process ---");
  
  // Step 1: Zero point calibration
  Serial.println("\nStep 1: Zero Point Calibration");
  Serial.println("Set sensor to minimum position/value and press any key...");
  waitForSerialInput();
  
  zeroPoint = analogRead(sensorPin);
  Serial.print("Zero point set to: ");
  Serial.println(zeroPoint);
  
  // Step 2: Maximum value calibration
  Serial.println("\nStep 2: Maximum Value Calibration");
  Serial.println("Set sensor to maximum position/value and press any key...");
  waitForSerialInput();
  
  int maxReading = analogRead(sensorPin);
  Serial.print("Maximum reading: ");
  Serial.println(maxReading);
  
  // Step 3: Set desired maximum output value
  Serial.println("\nStep 3: Set Desired Maximum Output Value");
  Serial.println("Enter the desired maximum output value (default=1023):");
  
  while (!Serial.available()) {
    // Wait for input
  }
  
  String input = Serial.readStringUntil('\n');
  if (input.length() > 0) {
    maxValue = input.toInt();
  }
  
  // Calculate scale factor
  scaleFactor = (float)maxValue / (maxReading - zeroPoint);
  
  // Save calibration data
  saveCalibrationData();
  
  Serial.println("\nCalibration completed and saved!");
  printCalibrationValues();
}

void saveCalibrationData() {
  calibration.setCalibrationValue("zero", zeroPoint);
  calibration.setCalibrationValue("scale", scaleFactor);
  calibration.setCalibrationValue("max", maxValue);
}

void loadCalibrationData() {
  calibration.getCalibrationValue("zero", zeroPoint);
  calibration.getCalibrationValue("scale", scaleFactor);
  calibration.getCalibrationValue("max", maxValue);
}

void printCalibrationValues() {
  Serial.println("\nCurrent Calibration Values:");
  Serial.print("Zero Point: ");
  Serial.println(zeroPoint);
  Serial.print("Scale Factor: ");
  Serial.println(scaleFactor, 6);
  Serial.print("Maximum Value: ");
  Serial.println(maxValue);
}

void waitForSerialInput() {
  while (Serial.available()) { Serial.read(); } // Clear buffer
  while (!Serial.available()) { delay(10); } // Wait for new input
  while (Serial.available()) { Serial.read(); } // Clear again
}