/*
  CalibrationBackupRestore.ino
  Example for CalibrationLib: Calibration Backup and Restore System

  This example demonstrates how to create a complete backup and restore system
  for calibration data. It provides functionality to:
  - Export calibration data in a portable format
  - Import calibration data from backup
  - Reset to default calibration values
  - Transfer calibration between devices
  - Backup calibration before firmware updates
  - Restore factory calibration settings

  Features:
  - Complete calibration data backup
  - CSV-format data export/import
  - Interactive serial menu interface
  - Default values restoration
  - Comprehensive error checking
  - Timeout protection for user input

  The example uses a 3-axis sensor calibration model with:
  - X, Y, Z axis offsets (integer values)
  - X, Y, Z axis scale factors (float values)
  - Device identification
  - Calibration timestamp

  Serial Menu Options:
  1: Backup calibration data
  2: Restore calibration data
  3: Reset to default values
  4: Print current calibration
  5: Exit program

  Data Format:
  <offsetX>,<offsetY>,<offsetZ>,<scaleX>,<scaleY>,<scaleZ>,<deviceName>,<date>
  Example: 10,15,20,0.95,0.98,1.02,Accelerometer,2023-05-15

  Hardware: ESP32 development board
  Software: Arduino IDE with ESP32 core, CalibrationLib

  Author: Judas Sithole (judassithole@duck.com)
  Created: 2025
  License: MIT
*/

#include <CalibrationLib.h>

CalibrationLib calibration;

// Example calibration values
struct CalibrationData {
  int offsetX;
  int offsetY;
  int offsetZ;
  float scaleX;
  float scaleY;
  float scaleZ;
  String deviceName;
  String lastCalibrationDate;
};

CalibrationData sensorData;

void setup() {
  Serial.begin(115200);
  delay(1000); // Give time to open serial monitor
  
  Serial.println("\nCalibration Backup and Restore Example");
  Serial.println("======================================\n");
  
  // Initialize the calibration library
  if (!calibration.begin("accel")) {
    Serial.println("Failed to initialize calibration library!");
    return;
  }
  
  // Check if we have calibration values already stored
  if (hasAllCalibrationValues()) {
    loadCalibrationData();
    Serial.println("Loaded existing calibration data:");
    printCalibrationData();
  } else {
    // Set some example calibration data
    setExampleCalibrationData();
    saveCalibrationData();
    Serial.println("Created and saved new calibration data:");
    printCalibrationData();
  }
  
  // Menu for backup and restore
  showMenu();
}

void loop() {
  if (Serial.available()) {
    char command = Serial.read();
    
    switch (command) {
      case '1':
        backupCalibration();
        break;
      case '2':
        Serial.println("\nEnter calibration data in the format:\n<offsetX>,<offsetY>,<offsetZ>,<scaleX>,<scaleY>,<scaleZ>,<deviceName>,<date>");
        Serial.println("Example: 10,15,20,0.95,0.98,1.02,Accelerometer,2023-05-15");
        break;
      case '3':
        resetToDefaults();
        break;
      case '4':
        printCalibrationData();
        break;
      case '5':
        calibration.end();
        Serial.println("\nProgram ended. Calibration data saved.");
        while(1); // Stop execution
        break;
      default:
        // Ignore other characters like newlines
        break;
    }
    
    // Check if we're waiting for restore data
    if (command == '2') {
      // Wait for the full line of data
      String dataLine = waitForSerialLine();
      if (dataLine.length() > 0) {
        restoreCalibration(dataLine);
      }
    }
    
    // Show menu again after command execution
    if (command >= '1' && command <= '5') {
      showMenu();
    }
  }
}

void showMenu() {
  Serial.println("\n--- Menu ---");
  Serial.println("1: Backup calibration data");
  Serial.println("2: Restore calibration data");
  Serial.println("3: Reset to default values");
  Serial.println("4: Print current calibration");
  Serial.println("5: Exit program");
  Serial.println("\nEnter your choice (1-5):");
}

bool hasAllCalibrationValues() {
  return calibration.hasCalibrationValue("offsetX") &&
         calibration.hasCalibrationValue("offsetY") &&
         calibration.hasCalibrationValue("offsetZ") &&
         calibration.hasCalibrationValue("scaleX") &&
         calibration.hasCalibrationValue("scaleY") &&
         calibration.hasCalibrationValue("scaleZ") &&
         calibration.hasCalibrationValue("deviceName") &&
         calibration.hasCalibrationValue("lastCalDate");
}

void loadCalibrationData() {
  calibration.getCalibrationValue("offsetX", sensorData.offsetX);
  calibration.getCalibrationValue("offsetY", sensorData.offsetY);
  calibration.getCalibrationValue("offsetZ", sensorData.offsetZ);
  calibration.getCalibrationValue("scaleX", sensorData.scaleX);
  calibration.getCalibrationValue("scaleY", sensorData.scaleY);
  calibration.getCalibrationValue("scaleZ", sensorData.scaleZ);
  calibration.getCalibrationValue("deviceName", sensorData.deviceName);
  calibration.getCalibrationValue("lastCalDate", sensorData.lastCalibrationDate);
}

void saveCalibrationData() {
  calibration.setCalibrationValue("offsetX", sensorData.offsetX);
  calibration.setCalibrationValue("offsetY", sensorData.offsetY);
  calibration.setCalibrationValue("offsetZ", sensorData.offsetZ);
  calibration.setCalibrationValue("scaleX", sensorData.scaleX);
  calibration.setCalibrationValue("scaleY", sensorData.scaleY);
  calibration.setCalibrationValue("scaleZ", sensorData.scaleZ);
  calibration.setCalibrationValue("deviceName", sensorData.deviceName.c_str());
  calibration.setCalibrationValue("lastCalDate", sensorData.lastCalibrationDate.c_str());
}

void setExampleCalibrationData() {
  sensorData.offsetX = 15;
  sensorData.offsetY = 22;
  sensorData.offsetZ = 5;
  sensorData.scaleX = 0.97;
  sensorData.scaleY = 1.02;
  sensorData.scaleZ = 0.99;
  sensorData.deviceName = "Accelerometer";
  sensorData.lastCalibrationDate = "2023-05-15";
}

void printCalibrationData() {
  Serial.println("\nCurrent Calibration Data:");
  Serial.print("Device: ");
  Serial.println(sensorData.deviceName);
  Serial.print("Last Calibration: ");
  Serial.println(sensorData.lastCalibrationDate);
  Serial.println("Offsets (X,Y,Z): " + 
                 String(sensorData.offsetX) + ", " + 
                 String(sensorData.offsetY) + ", " + 
                 String(sensorData.offsetZ));
  Serial.println("Scales (X,Y,Z): " + 
                 String(sensorData.scaleX, 4) + ", " + 
                 String(sensorData.scaleY, 4) + ", " + 
                 String(sensorData.scaleZ, 4));
}

void backupCalibration() {
  Serial.println("\n--- Calibration Backup Data ---");
  Serial.print(sensorData.offsetX);
  Serial.print(",");
  Serial.print(sensorData.offsetY);
  Serial.print(",");
  Serial.print(sensorData.offsetZ);
  Serial.print(",");
  Serial.print(sensorData.scaleX, 6);
  Serial.print(",");
  Serial.print(sensorData.scaleY, 6);
  Serial.print(",");
  Serial.print(sensorData.scaleZ, 6);
  Serial.print(",");
  Serial.print(sensorData.deviceName);
  Serial.print(",");
  Serial.println(sensorData.lastCalibrationDate);
  Serial.println("--- End of Backup Data ---");
  Serial.println("\nCopy the line above to restore calibration later.");
}

void restoreCalibration(String dataLine) {
  // Parse the comma-separated values
  int values[3];
  float scales[3];
  String strings[2];
  
  int commaIndex = 0;
  int nextCommaIndex = 0;
  
  // Parse the integers (offsets)
  for (int i = 0; i < 3; i++) {
    nextCommaIndex = dataLine.indexOf(',', commaIndex);
    if (nextCommaIndex == -1) {
      Serial.println("Error: Invalid data format");
      return;
    }
    values[i] = dataLine.substring(commaIndex, nextCommaIndex).toInt();
    commaIndex = nextCommaIndex + 1;
  }
  
  // Parse the floats (scales)
  for (int i = 0; i < 3; i++) {
    nextCommaIndex = dataLine.indexOf(',', commaIndex);
    if (nextCommaIndex == -1) {
      Serial.println("Error: Invalid data format");
      return;
    }
    scales[i] = dataLine.substring(commaIndex, nextCommaIndex).toFloat();
    commaIndex = nextCommaIndex + 1;
  }
  
  // Parse the strings (device name and date)
  for (int i = 0; i < 1; i++) { // Only get the device name this way
    nextCommaIndex = dataLine.indexOf(',', commaIndex);
    if (nextCommaIndex == -1) {
      Serial.println("Error: Invalid data format");
      return;
    }
    strings[i] = dataLine.substring(commaIndex, nextCommaIndex);
    commaIndex = nextCommaIndex + 1;
  }
  
  // The last field is the date (everything after the last comma)
  strings[1] = dataLine.substring(commaIndex);
  
  // Update the calibration data
  sensorData.offsetX = values[0];
  sensorData.offsetY = values[1];
  sensorData.offsetZ = values[2];
  sensorData.scaleX = scales[0];
  sensorData.scaleY = scales[1];
  sensorData.scaleZ = scales[2];
  sensorData.deviceName = strings[0];
  sensorData.lastCalibrationDate = strings[1];
  
  // Save to flash
  saveCalibrationData();
  
  Serial.println("\nCalibration data restored and saved!");
  printCalibrationData();
}

void resetToDefaults() {
  Serial.println("\nResetting to default calibration values...");
  setExampleCalibrationData();
  saveCalibrationData();
  Serial.println("Reset complete!");
  printCalibrationData();
}

String waitForSerialLine() {
  String line = "";
  unsigned long startTime = millis();
  const unsigned long timeout = 30000; // 30 seconds timeout
  
  while (millis() - startTime < timeout) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n') {
        break;
      }
      line += c;
      startTime = millis(); // Reset timeout when receiving data
    }
    yield(); // Allow ESP32 to handle background tasks
  }
  
  return line;
}