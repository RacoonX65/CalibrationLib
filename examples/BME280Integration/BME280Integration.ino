#include <Wire.h>
#include <Adafruit_BME280.h>
#include <CalibrationLib.h>

Adafruit_BME280 bme;
CalibrationLib calib;

// Calibration keys
const char* TEMP_OFFSET_KEY = "temp_offset";
const char* PRESSURE_OFFSET_KEY = "pres_offset";
const char* HUMIDITY_OFFSET_KEY = "hum_offset";

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // Initialize BME280
  if (!bme.begin(0x76)) {
    Serial.println("BME280 not found!");
    while (1);
  }
  
  // Initialize calibration with BME280 namespace
  calib.begin("bme280");
  
  // Load or set default calibration values
  float tempOffset, pressOffset, humOffset;
  calib.getCalibrationValue(TEMP_OFFSET_KEY, tempOffset, -2.0f);  // Default offset of -2.0°C
  calib.getCalibrationValue(PRESSURE_OFFSET_KEY, pressOffset, 0.0f);
  calib.getCalibrationValue(HUMIDITY_OFFSET_KEY, humOffset, -5.0f); // Default offset of -5%
  
  Serial.println("Current calibration values:");
  Serial.printf("Temperature offset: %.2f°C\n", tempOffset);
  Serial.printf("Pressure offset: %.2f hPa\n", pressOffset);
  Serial.printf("Humidity offset: %.2f%%\n", humOffset);
}

void loop() {
  // Read raw values
  float rawTemp = bme.readTemperature();
  float rawPressure = bme.readPressure() / 100.0F;
  float rawHumidity = bme.readHumidity();
  
  // Apply calibration offsets
  float tempOffset, pressOffset, humOffset;
  calib.getCalibrationValue(TEMP_OFFSET_KEY, tempOffset, 0.0f);
  calib.getCalibrationValue(PRESSURE_OFFSET_KEY, pressOffset, 0.0f);
  calib.getCalibrationValue(HUMIDITY_OFFSET_KEY, humOffset, 0.0f);
  
  float calibratedTemp = rawTemp + tempOffset;
  float calibratedPressure = rawPressure + pressOffset;
  float calibratedHumidity = rawHumidity + humOffset;
  
  // Print results
  Serial.printf("Temperature: %.2f°C (raw) / %.2f°C (calibrated)\n", 
                rawTemp, calibratedTemp);
  Serial.printf("Pressure: %.2f hPa (raw) / %.2f hPa (calibrated)\n", 
                rawPressure, calibratedPressure);
  Serial.printf("Humidity: %.2f%% (raw) / %.2f%% (calibrated)\n", 
                rawHumidity, calibratedHumidity);
  
  delay(2000);
}