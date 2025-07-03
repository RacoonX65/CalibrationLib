/*
  MultiSensorCalibration.ino
  Example for CalibrationLib: Multiple Sensor Calibration Management

  This example demonstrates how to efficiently manage calibration data for multiple
  sensors using CalibrationLib's namespace feature. It shows how to:
  - Organize calibration data for different sensors
  - Load and store calibration parameters per sensor
  - Apply calibration to sensor readings in real-time

  Features:
  - Independent calibration storage for each sensor
  - Namespace-based organization
  - Default calibration fallback
  - Real-time calibration application
  - Persistent storage across power cycles

  Calibration Parameters per Sensor:
  - offset: Zero-point correction value
  - scale: Multiplication factor for scaling
  - unit: Measurement unit string

  Supported Sensors:
  - Temperature (Pin 36)
    Default: offset=5, scale=0.1, unit=C
  - Pressure (Pin 39)
    Default: offset=10, scale=0.01, unit=hPa
  - Humidity (Pin 34)
    Default: offset=2, scale=0.05, unit=%

  Hardware Setup:
  - ESP32 development board
  - Analog temperature sensor on GPIO36
  - Analog pressure sensor on GPIO39
  - Analog humidity sensor on GPIO34

  Serial Output Format:
  [SensorName]: Raw = [RawValue], Calibrated = [CalibratedValue] [Unit]

  Dependencies:
  - ESP32 Arduino Core
  - CalibrationLib

  Note: This example uses simulated sensor readings.
  For real applications, replace analogRead() with
  actual sensor reading implementations.

  Author: Judas Sithole (judassithole@duck.com)
  Created: 2025
  License: MIT
*/

#include <CalibrationLib.h>

// Create a single instance of the calibration library
CalibrationLib calibration;

// Simulated sensor pins
const int temperatureSensorPin = 36;
const int pressureSensorPin = 39;
const int humiditySensorPin = 34;

// Calibration parameters for each sensor
struct SensorCalibration {
  int offset;
  float scale;
  String unit;
};

SensorCalibration tempSensor;
SensorCalibration pressureSensor;
SensorCalibration humiditySensor;

void setup() {
  Serial.begin(115200);
  delay(1000); // Give time to open serial monitor
  
  Serial.println("\nMulti-Sensor Calibration Example");
  Serial.println("================================\n");
  
  // Load calibration for temperature sensor
  loadSensorCalibration("temp", tempSensor, 5, 0.1, "C");
  
  // Load calibration for pressure sensor
  loadSensorCalibration("press", pressureSensor, 10, 0.01, "hPa");
  
  // Load calibration for humidity sensor
  loadSensorCalibration("humid", humiditySensor, 2, 0.05, "%");
  
  // Print all calibration values
  Serial.println("\nAll Sensors Calibration Data:");
  printSensorCalibration("Temperature", tempSensor);
  printSensorCalibration("Pressure", pressureSensor);
  printSensorCalibration("Humidity", humiditySensor);
  
  Serial.println("\nReading sensor values...");
}

void loop() {
  // Read and display values from all sensors
  readAndDisplaySensor("Temperature", temperatureSensorPin, tempSensor);
  readAndDisplaySensor("Pressure", pressureSensorPin, pressureSensor);
  readAndDisplaySensor("Humidity", humiditySensorPin, humiditySensor);
  
  Serial.println("----------------------------");
  delay(2000);
}

void loadSensorCalibration(const char* sensorName, SensorCalibration &sensorCal, 
                          int defaultOffset, float defaultScale, const char* defaultUnit) {
  // Initialize the calibration library with the sensor's namespace
  calibration.begin(sensorName);
  
  // Check if calibration exists
  if (calibration.hasCalibrationValue("offset") && 
      calibration.hasCalibrationValue("scale") &&
      calibration.hasCalibrationValue("unit")) {
    
    // Load existing calibration
    calibration.getCalibrationValue("offset", sensorCal.offset);
    calibration.getCalibrationValue("scale", sensorCal.scale);
    calibration.getCalibrationValue("unit", sensorCal.unit);
    
    Serial.print("Loaded existing calibration for ");
    Serial.println(sensorName);
  } else {
    // Set default calibration
    sensorCal.offset = defaultOffset;
    sensorCal.scale = defaultScale;
    sensorCal.unit = defaultUnit;
    
    // Save the default calibration
    calibration.setCalibrationValue("offset", sensorCal.offset);
    calibration.setCalibrationValue("scale", sensorCal.scale);
    calibration.setCalibrationValue("unit", sensorCal.unit.c_str());
    
    Serial.print("Created new calibration for ");
    Serial.println(sensorName);
  }
  
  // Close this namespace before opening another
  calibration.end();
}

void printSensorCalibration(const char* sensorName, const SensorCalibration &sensorCal) {
  Serial.print(sensorName);
  Serial.print(" Sensor: Offset = ");
  Serial.print(sensorCal.offset);
  Serial.print(", Scale = ");
  Serial.print(sensorCal.scale, 4);
  Serial.print(", Unit = ");
  Serial.println(sensorCal.unit);
}

void readAndDisplaySensor(const char* sensorName, int sensorPin, const SensorCalibration &sensorCal) {
  // Read raw value
  int rawValue = analogRead(sensorPin);
  
  // Apply calibration
  float calibratedValue = (rawValue - sensorCal.offset) * sensorCal.scale;
  
  // Display results
  Serial.print(sensorName);
  Serial.print(": Raw = ");
  Serial.print(rawValue);
  Serial.print(", Calibrated = ");
  Serial.print(calibratedValue, 2);
  Serial.print(" ");
  Serial.println(sensorCal.unit);
}