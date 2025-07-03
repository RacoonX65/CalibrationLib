/*
  OLEDCalibration.ino
  Example for CalibrationLib: OLED Display Settings Management

  This example demonstrates how to use CalibrationLib to store and manage
  display settings for an SSD1306 OLED display. It shows how to:
  - Save and load display preferences
  - Adjust display parameters via serial commands
  - Persist display settings across power cycles

  Features:
  - Persistent storage of display settings
  - Real-time parameter adjustment
  - Serial command interface
  - Settings visualization on OLED
  - Default values fallback

  Calibration Parameters:
  - Brightness (0-255, default: 255)
  - Contrast (0-255, default: 128)
  - Rotation (0-3, default: 0)
    0: Normal
    1: 90° clockwise
    2: 180°
    3: 270° clockwise

  Hardware Setup:
  - ESP32 development board
  - SSD1306 OLED display (128x64)
    - Connected via I2C (address 0x3C)
    - SDA: GPIO21 (default)
    - SCL: GPIO22 (default)

  Serial Commands:
  brightness=[0-255] : Set display brightness
  contrast=[0-255]  : Set display contrast
  rotation=[0-3]    : Set display rotation

  Dependencies:
  - ESP32 Arduino Core
  - Adafruit GFX Library
  - Adafruit SSD1306 Library
  - CalibrationLib

  Note: Some OLED displays may not support brightness control.
  In such cases, only contrast adjustment will be effective.

  Author: Judas Sithole (judassithole@duck.com)
  Created: 2025
  License: MIT
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <CalibrationLib.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
CalibrationLib calib;

// Calibration keys
const char* BRIGHTNESS_KEY = "brightness";
const char* CONTRAST_KEY = "contrast";
const char* ROTATION_KEY = "rotation";

void setup() {
  Serial.begin(115200);
  
  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while(1);
  }
  
  // Initialize calibration
  calib.begin("oled");
  
  // Load display settings from calibration
  int brightness, contrast, rotation;
  calib.getCalibrationValue(BRIGHTNESS_KEY, brightness, 255);  // Default full brightness
  calib.getCalibrationValue(CONTRAST_KEY, contrast, 128);     // Default medium contrast
  calib.getCalibrationValue(ROTATION_KEY, rotation, 0);       // Default rotation
  
  // Apply settings
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(contrast);
  display.setRotation(rotation);
  
  // Show current settings
  displaySettings(brightness, contrast, rotation);
}

void displaySettings(int brightness, int contrast, int rotation) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  
  display.println("Display Settings:");
  display.printf("Brightness: %d\n", brightness);
  display.printf("Contrast: %d\n", contrast);
  display.printf("Rotation: %d\n", rotation);
  
  display.display();
}

void loop() {
  // Check Serial for calibration commands
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    if (cmd.startsWith("brightness=")) {
      int val = cmd.substring(11).toInt();
      calib.setCalibrationValue(BRIGHTNESS_KEY, val);
    }
    else if (cmd.startsWith("contrast=")) {
      int val = cmd.substring(9).toInt();
      calib.setCalibrationValue(CONTRAST_KEY, val);
      display.ssd1306_command(SSD1306_SETCONTRAST);
      display.ssd1306_command(val);
    }
    else if (cmd.startsWith("rotation=")) {
      int val = cmd.substring(9).toInt();
      calib.setCalibrationValue(ROTATION_KEY, val);
      display.setRotation(val);
    }
    
    // Update display
    int brightness, contrast, rotation;
    calib.getCalibrationValue(BRIGHTNESS_KEY, brightness, 255);
    calib.getCalibrationValue(CONTRAST_KEY, contrast, 128);
    calib.getCalibrationValue(ROTATION_KEY, rotation, 0);
    displaySettings(brightness, contrast, rotation);
  }
  
  delay(100);
}