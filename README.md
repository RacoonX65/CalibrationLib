# CalibrationLib

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/RacoonX65/CalibrationLib)
[![Platform](https://img.shields.io/badge/platform-esp32-green.svg)](https://github.com/RacoonX65/CalibrationLib)
[![License](https://img.shields.io/badge/license-MIT-yellow.svg)](LICENSE)

A versatile Arduino library for ESP32 that provides persistent calibration data storage and management capabilities. This library simplifies the process of storing, retrieving, and managing calibration values for sensors, displays, and communication settings.

## Table of Contents
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Installation](#installation)
- [Dependencies](#dependencies)
- [Calibration Concepts](#calibration-concepts)
- [Basic Usage](#basic-usage)
- [Examples](#examples)
- [API Reference](#api-reference)
- [Error Handling](#error-handling)
- [Best Practices](#best-practices)
- [Contributing](#contributing)
- [License](#license)
- [Support](#support)

## Features

- Persistent storage using ESP32's NVS (Non-Volatile Storage)
- Multiple data type support (int, float, string)
- Namespace-based organization
- JSON import/export capabilities
- Version control and timestamp management
- Encryption support for sensitive data
- Batch operations for atomic updates
- Comprehensive error handling and debugging
- Memory usage monitoring
- Real-time validation

## Hardware Requirements

- ESP32 development board (any variant)
- Minimum 4MB flash memory
- Compatible sensors (optional, based on examples):
  - BME280 (temperature/humidity/pressure)
  - MPU6050 (accelerometer/gyroscope)
  - Potentiometer
  - RGB LED
  - OLED Display

## Installation

### Method 1: Arduino Library Manager
1. Open Arduino IDE
2. Go to `Sketch > Include Library > Manage Libraries`
3. Search for "CalibrationLib"
4. Click Install

### Method 2: Manual Installation
1. Download this repository as a ZIP file
2. In Arduino IDE, go to `Sketch > Include Library > Add .ZIP Library`
3. Select the downloaded ZIP file

## Dependencies

- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- Preferences.h (included with ESP32 core)
- [ArduinoJson](https://arduinojson.org/) (for JSON functionality)
- [Adafruit BME280 Library](https://github.com/adafruit/Adafruit_BME280_Library) (for BME280 example)
- [Adafruit MPU6050](https://github.com/adafruit/Adafruit_MPU6050) (for sensor fusion example)
- [Adafruit Unified Sensor](https://github.com/adafruit/Adafruit_Sensor) (sensor support)

## Calibration Concepts

### Understanding Calibration
Calibration is the process of adjusting measurement data to match known reference values. Common calibration types include:

1. **Linear Calibration**
   ```cpp
   calibrated_value = (raw_value * scale) + offset
   ```

2. **Multi-point Calibration**
   - Uses multiple reference points
   - Interpolates between known values
   - Better accuracy across range

3. **Auto-calibration**
   - Self-adjusting based on known conditions
   - Periodic recalibration support

### Data Persistence
Calibration data is stored in ESP32's Non-Volatile Storage (NVS):
- Survives power cycles
- Organized by namespaces
- Protected against corruption
- Optional encryption

## Basic Usage

```cpp
#include <CalibrationLib.h>

CalibrationLib calib;

void setup() {
  // Initialize with a namespace
  calib.begin("mysensor");
  
  // Store calibration values
  calib.setCalibrationValue("offset", 2.5f);
  calib.setCalibrationValue("scale", 1.023f);
  
  // Read calibration values
  float offset, scale;
  calib.getCalibrationValue("offset", offset, 0.0f);  // Default 0.0 if not found
  calib.getCalibrationValue("scale", scale, 1.0f);    // Default 1.0 if not found
  
  // Export to JSON
  String jsonData;
  calib.exportToJson(jsonData);
  
  // Close when done
  calib.end();
}
```

### JSON Format Example
```json
{
  "version": "1.0.0",
  "timestamp": 1634567890,
  "namespace": "mysensor",
  "values": {
    "offset": 2.5,
    "scale": 1.023
  }
}
```

## Examples

### Basic Examples
- **BasicCalibration**: Simple demonstration of storing and retrieving values
- **CalibrationBackupRestore**: Data backup and restore functionality
- **MultiSensorCalibration**: Managing multiple sensor calibrations
- **SensorCalibrationWorkflow**: Step-by-step calibration process

### Sensor Integration
- **BME280Integration**: Environmental sensor calibration
- **TemperatureAutoCalibration**: Self-calibrating temperature sensor
- **PotentiometerCalibration**: Analog input calibration
- **RGBLedCalibration**: LED color balance adjustment

### Communication & Display
- **BLECalibration**: Bluetooth configuration interface
- **MQTTCalibration**: MQTT settings management
- **OLEDCalibration**: Display parameter adjustment
- **WebCalibrationInterface**: Browser-based calibration

### Advanced Features
- **SensorFusion**: Combined sensor data calibration
- **UnitTests**: Library validation tests

## Error Handling

### Error Codes
```cpp
CAL_OK               // Operation successful
CAL_NOT_INITIALIZED  // Library not initialized
CAL_INVALID_PARAM    // Invalid parameter provided
CAL_WRITE_ERROR      // Failed to write to storage
CAL_READ_ERROR       // Failed to read from storage
CAL_MEMORY_ERROR     // Memory allocation failed
CAL_ENCRYPTION_ERROR // Encryption/decryption failed
```

### Debug Levels
```cpp
DEBUG_NONE    // No debug output
DEBUG_ERROR   // Errors only
DEBUG_INFO    // General information
DEBUG_VERBOSE // Detailed debugging
```

## Best Practices

1. **Namespace Organization**
   - Use descriptive namespace names
   - Separate concerns (e.g., "sensor1", "display")
   - Keep names short but meaningful

2. **Error Handling**
   - Check return values
   - Set appropriate debug levels
   - Provide fallback values

3. **Resource Management**
   - Close calibration when done
   - Monitor memory usage
   - Use batch operations for multiple updates

4. **Security**
   - Enable encryption for sensitive data
   - Validate input data
   - Regular backups

5. **Version Control**
   - Track calibration versions
   - Handle version migrations
   - Set expiration times

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details

## Support

1. Check the examples included with the library
2. Review the documentation
3. Open an issue on GitHub
4. Contact: judassithole@duck.com

## Acknowledgments

- ESP32 Arduino Core team
- Arduino community
- All contributors
```
