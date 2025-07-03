/*
  SensorFusion.ino
  Example for CalibrationLib: Multi-Sensor Fusion with BME280 and MPU6050

  This example demonstrates how to use CalibrationLib to manage calibration
  data for multiple sensors simultaneously, combining environmental and
  motion sensing. It shows how to:
  - Calibrate multiple sensors in a single workflow
  - Store complex calibration data using JSON
  - Apply calibration corrections in real-time
  - Version and timestamp calibration data

  Features:
  - Multi-sensor calibration management
  - JSON-based array storage
  - Automatic offset calculation
  - Real-time calibration application
  - Calibration versioning
  - Timestamp tracking
  - Debug level control

  Sensors:
  1. BME280 Environmental Sensor
     - Temperature (°C)
     - Humidity (%)
     - Pressure (hPa)
  2. MPU6050 Motion Sensor
     - Accelerometer (m/s²)
     - Gyroscope (rad/s)

  Calibration Process:
  - Collects 100 samples from each sensor
  - Calculates offsets against reference values:
    * Temperature: 25°C
    * Humidity: 50%
    * Pressure: 1013.25 hPa
    * Accelerometer: 0 m/s² (gravity compensated)
    * Gyroscope: 0 rad/s

  Hardware Setup:
  - ESP32 development board
  - BME280 sensor (I2C)
  - MPU6050 sensor (I2C)
  - Connections:
    * SDA: GPIO21 (default)
    * SCL: GPIO22 (default)

  Serial Interface:
  - Baud Rate: 115200
  - Command 'c': Start calibration process
  - Output Format:
    * Environmental: Temperature, Humidity, Pressure
    * Motion: Acceleration (XYZ), Gyroscope (XYZ)

  Dependencies:
  - ESP32 Arduino Core
  - Wire Library
  - Adafruit BME280 Library
  - Adafruit MPU6050 Library
  - ArduinoJson Library
  - CalibrationLib

  Note: Place sensors in a stable position during
  calibration for accurate offset calculations.

  Author: Judas Sithole (judassithole@duck.com)
  Created: 2025
  License: MIT
*/

#include <CalibrationLib.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>

CalibrationLib calibration;
Adafruit_BME280 bme;
Adafruit_MPU6050 mpu;

// Calibration offsets
float tempOffset = 0.0f;
float humidityOffset = 0.0f;
float pressureOffset = 0.0f;
float accelOffset[3] = {0.0f, 0.0f, 0.0f};
float gyroOffset[3] = {0.0f, 0.0f, 0.0f};

void loadCalibration() {
    calibration.getCalibrationValue("temp_offset", tempOffset);
    calibration.getCalibrationValue("humidity_offset", humidityOffset);
    calibration.getCalibrationValue("pressure_offset", pressureOffset);
    
    String jsonOffsets;
    if (calibration.getCalibrationValue("accel_offset", jsonOffsets)) {
        StaticJsonDocument<200> doc;
        deserializeJson(doc, jsonOffsets);
        accelOffset[0] = doc[0];
        accelOffset[1] = doc[1];
        accelOffset[2] = doc[2];
    }
    
    if (calibration.getCalibrationValue("gyro_offset", jsonOffsets)) {
        StaticJsonDocument<200> doc;
        deserializeJson(doc, jsonOffsets);
        gyroOffset[0] = doc[0];
        gyroOffset[1] = doc[1];
        gyroOffset[2] = doc[2];
    }
}

void saveCalibration() {
    calibration.setCalibrationValue("temp_offset", tempOffset);
    calibration.setCalibrationValue("humidity_offset", humidityOffset);
    calibration.setCalibrationValue("pressure_offset", pressureOffset);
    
    StaticJsonDocument<200> doc;
    String jsonString;
    
    // Save accelerometer offsets
    doc.clear();
    doc.add(accelOffset[0]);
    doc.add(accelOffset[1]);
    doc.add(accelOffset[2]);
    serializeJson(doc, jsonString);
    calibration.setCalibrationValue("accel_offset", jsonString.c_str());
    
    // Save gyroscope offsets
    doc.clear();
    doc.add(gyroOffset[0]);
    doc.add(gyroOffset[1]);
    doc.add(gyroOffset[2]);
    serializeJson(doc, jsonString);
    calibration.setCalibrationValue("gyro_offset", jsonString.c_str());
    
    // Set version and timestamp
    calibration.setCalibrationVersion("1.0");
    calibration.setCalibrationTimestamp();
}

void calibrateSensors() {
    Serial.println("Starting sensor calibration...");
    
    // Collect multiple samples for averaging
    const int samples = 100;
    float temp_sum = 0, humidity_sum = 0, pressure_sum = 0;
    float accel_sum[3] = {0}, gyro_sum[3] = {0};
    
    for (int i = 0; i < samples; i++) {
        temp_sum += bme.readTemperature();
        humidity_sum += bme.readHumidity();
        pressure_sum += bme.readPressure() / 100.0F;
        
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);
        accel_sum[0] += a.acceleration.x;
        accel_sum[1] += a.acceleration.y;
        accel_sum[2] += a.acceleration.z - 9.81; // Remove gravity
        gyro_sum[0] += g.gyro.x;
        gyro_sum[1] += g.gyro.y;
        gyro_sum[2] += g.gyro.z;
        
        delay(10);
    }
    
    // Calculate offsets
    tempOffset = 25.0 - (temp_sum / samples); // Assuming 25°C as reference
    humidityOffset = 50.0 - (humidity_sum / samples); // Assuming 50% as reference
    pressureOffset = 1013.25 - (pressure_sum / samples); // Assuming 1013.25 hPa as reference
    
    for (int i = 0; i < 3; i++) {
        accelOffset[i] = -(accel_sum[i] / samples);
        gyroOffset[i] = -(gyro_sum[i] / samples);
    }
    
    saveCalibration();
    Serial.println("Calibration complete!");
}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    
    if (!bme.begin()) {
        Serial.println("Could not find BME280 sensor!");
        return;
    }
    
    if (!mpu.begin()) {
        Serial.println("Could not find MPU6050 sensor!");
        return;
    }
    
    calibration.begin("sensor_fusion");
    calibration.setDebugLevel(DEBUG_INFO);
    
    loadCalibration();
    
    Serial.println("Enter 'c' to calibrate sensors");
}

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();
        if (cmd == 'c') {
            calibrateSensors();
        }
    }
    
    // Read and apply calibration
    float temperature = bme.readTemperature() + tempOffset;
    float humidity = bme.readHumidity() + humidityOffset;
    float pressure = (bme.readPressure() / 100.0F) + pressureOffset;
    
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    float accelX = a.acceleration.x + accelOffset[0];
    float accelY = a.acceleration.y + accelOffset[1];
    float accelZ = a.acceleration.z + accelOffset[2];
    
    float gyroX = g.gyro.x + gyroOffset[0];
    float gyroY = g.gyro.y + gyroOffset[1];
    float gyroZ = g.gyro.z + gyroOffset[2];
    
    // Print calibrated values
    Serial.printf("Temperature: %.2f°C, Humidity: %.2f%%, Pressure: %.2fhPa\n",
                  temperature, humidity, pressure);
    Serial.printf("Accel X: %.2f, Y: %.2f, Z: %.2f m/s²\n",
                  accelX, accelY, accelZ);
    Serial.printf("Gyro X: %.2f, Y: %.2f, Z: %.2f rad/s\n\n",
                  gyroX, gyroY, gyroZ);
    
    delay(1000);
}