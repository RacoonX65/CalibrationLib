/*
  BasicCalibration.ino
  Example for CalibrationLib: Basic Calibration Example

  This example demonstrates the fundamental usage of CalibrationLib for
  managing sensor calibration data in ESP32's non-volatile storage.

  Features:
  - Basic calibration data management
  - Multiple data type support (int, float, string)
  - Persistent storage across power cycles
  - Default value fallback
  - Real-time calibration application
  - Error handling

  Calibration Parameters:
  1. Sensor Offset (int)
     - Corrects zero-point error
     - Default: 10

  2. Scale Factor (float)
     - Adjusts measurement sensitivity
     - Default: 0.95

  3. Sensor Name (string)
     - Identifies the sensor
     - Default: "Temperature Sensor"

  Calibration Formula:
  calibratedValue = (rawValue - offset) * scaleFactor

  Hardware Setup:
  - ESP32 development board
  - Serial connection (115200 baud)

  Serial Output Format:
  - Initialization status
  - Current calibration values
  - Example raw and calibrated readings

  Storage:
  - Namespace: "mysensor"
  - Keys: "offset", "scale", "name"

  Dependencies:
  - ESP32 Arduino Core
  - CalibrationLib
  - Preferences library (included with ESP32 core)

  Note: This is a basic example intended for learning
  the fundamental concepts of sensor calibration and
  persistent storage. For production use, consider
  adding error checking and validation.

  Author: Judas Sithole (judassithole@duck.com)
  Created: 2025
  License: MIT
*/


#include <CalibrationLib.h>

CalibrationLib calibration;

// Example calibration values
int sensorOffset = 0;
float scaleFactor = 1.0;
String sensorName = "";

void setup() {
  Serial.begin(115200);
  delay(1000); // Give time to open serial monitor
  
  Serial.println("ESP32 Calibration Library Example");
  Serial.println("==================================");
  
  // Initialize the calibration library
  if (!calibration.begin("mysensor")) {
    Serial.println("Failed to initialize calibration library!");
    return;
  }
  
  // Check if we have calibration values already stored
  if (calibration.hasCalibrationValue("offset") && 
      calibration.hasCalibrationValue("scale") &&
      calibration.hasCalibrationValue("name")) {
    
    // Load existing calibration values by reference
    calibration.getCalibrationValue("offset", sensorOffset);
    calibration.getCalibrationValue("scale", scaleFactor);
    calibration.getCalibrationValue("name", sensorName);
    
    Serial.println("Loaded existing calibration values:");
  } else {
    // Set default calibration values
    sensorOffset = 10;
    scaleFactor = 0.95;
    sensorName = "Temperature Sensor";
    
    // Save the calibration values
    calibration.setCalibrationValue("offset", sensorOffset);
    calibration.setCalibrationValue("scale", scaleFactor);
    calibration.setCalibrationValue("name", sensorName.c_str());
    
    Serial.println("Saved new calibration values:");
  }
  
  // Print the current calibration values
  Serial.print("Sensor Name: ");
  Serial.println(sensorName);
  Serial.print("Offset: ");
  Serial.println(sensorOffset);
  Serial.print("Scale Factor: ");
  Serial.println(scaleFactor, 4);
  
  // Example of using the calibration values
  int rawValue = 100; // Example raw sensor reading
  float calibratedValue = (rawValue - sensorOffset) * scaleFactor;
  
  Serial.print("\nRaw sensor value: ");
  Serial.println(rawValue);
  Serial.print("Calibrated value: ");
  Serial.println(calibratedValue, 2);
  
  // Close the calibration library
  calibration.end();
}

void loop() {
  // Nothing to do here
}