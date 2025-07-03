


          
# CalibrationLib

A versatile Arduino library for ESP32 that provides persistent calibration data storage and management capabilities. This library simplifies the process of storing, retrieving, and managing calibration values for sensors, displays, and communication settings.

## Features

- Persistent storage of calibration values in ESP32's NVS (Non-Volatile Storage)
- Support for multiple data types (int, float, string)
- Namespace-based organization for different calibration sets
- Easy-to-use API for reading and writing calibration values
- Comprehensive examples for various use cases

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

- ESP32 Arduino Core
- Preferences.h (included with ESP32 core)
- ArduinoJson (for JSON import/export functionality)

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
  
  // Close when done
  calib.end();
}
```

## Examples

The library includes several example sketches demonstrating different use cases:

### Basic Examples
- **BasicCalibration**: Simple demonstration of storing and retrieving calibration values
- **CalibrationBackupRestore**: Backup and restore calibration data
- **MultiSensorCalibration**: Managing calibration for multiple sensors
- **SensorCalibrationWorkflow**: Step-by-step calibration process

### Sensor Integration
- **BME280Integration**: Calibration for BME280 temperature/humidity/pressure sensor
- **TemperatureAutoCalibration**: Automated temperature sensor calibration

### Display Integration
- **OLEDCalibration**: Display settings calibration for OLED screens

### Communication Integration
- **MQTTCalibration**: MQTT broker settings management
- **BLECalibration**: Bluetooth Low Energy configuration
- **WebCalibrationInterface**: Web interface for calibration management

## API Reference

### Constructor
```cpp
CalibrationLib();
```

### Basic Methods
```cpp
bool begin(const char* namespace_name = "calib");  // Initialize with namespace
void end();                                       // Close and save
```

### Setting Values
```cpp
bool setCalibrationValue(const char* key, int value);
bool setCalibrationValue(const char* key, float value);
bool setCalibrationValue(const char* key, const char* value);
```

### Getting Values
```cpp
bool getCalibrationValue(const char* key, int& value, int defaultValue = 0);
bool getCalibrationValue(const char* key, float& value, float defaultValue = 0.0f);
bool getCalibrationValue(const char* key, String& value, const char* defaultValue = "");
```

### Utility Methods
```cpp
bool hasCalibrationValue(const char* key);        // Check if value exists
bool removeCalibrationValue(const char* key);      // Remove specific value
bool clearAllCalibrationValues();                 // Clear all values
```

## Best Practices

1. **Namespace Organization**
   - Use descriptive namespace names for different devices or sensors
   - Keep namespace names short but meaningful

2. **Error Handling**
   - Always check return values from set/get operations
   - Provide meaningful default values for get operations

3. **Resource Management**
   - Call `end()` when finished with calibration operations
   - Don't keep the calibration storage open unnecessarily

4. **Data Persistence**
   - Remember that values persist across power cycles
   - Use `clearAllCalibrationValues()` when needed to reset to defaults

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details

## Support

For support, please:
1. Check the examples included with the library
2. Review the documentation
3. Open an issue on GitHub

## Acknowledgments

- ESP32 Arduino Core team for the Preferences library
- Arduino community for testing and feedback
- Contributors who have helped improve the library
        