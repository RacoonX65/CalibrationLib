/*
  BLECalibration.ino
  Example for CalibrationLib: BLE Remote Calibration

  This example demonstrates wireless sensor calibration using
  Bluetooth Low Energy (BLE) on an ESP32 device, enabling
  remote calibration via a smartphone app.

  Features:
  - Wireless calibration parameter updates
  - JSON-formatted data streaming
  - Automatic reconnection handling
  - Persistent calibration storage
  - Real-time sensor value updates
  - Error handling and validation

  Calibration Parameters:
  1. Offset (int)
     - Zero-point correction
     - Default: 0

  2. Scale Factor (float)
     - Sensitivity adjustment
     - Default: 1.0

  3. Sensor Name (string)
     - Device identifier
     - Default: "BLE Sensor"

  BLE Configuration:
  - Service UUID: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
  - Characteristic UUID: beb5483e-36e1-4688-b7f5-ea07361b26a8
  - Properties: Read, Write, Notify, Indicate

  BLE Commands:
  1. Set Commands (format: "CMD:VALUE")
     - NAME:value  - Set sensor name
     - OFFSET:n    - Set calibration offset
     - SCALE:n.n   - Set calibration scale factor
     - GET         - Request current values

  JSON Output Format:
  {"raw": rawValue, "cal": calibratedValue}

  Calibration Formula:
  calibratedValue = (rawValue - offset) * scale

  Hardware Setup:
  - ESP32 development board
  - Analog sensor on GPIO36 (ADC1_0)
  - Smartphone with BLE capability
  - Serial connection (115200 baud)

  Storage:
  - Namespace: "blesensor"
  - Keys: "offset", "scale", "name"

  Dependencies:
  - ESP32 Arduino Core
  - BLE libraries:
    * BLEDevice
    * BLEServer
    * BLEUtils
    * BLE2902
  - CalibrationLib

  Note: This example assumes basic knowledge of BLE
  communication. For production use, consider adding
  security features and input validation.

  Author: Judas Sithole (judassithole@duck.com)
  Created: 2025
  License: MIT
*/


#include <CalibrationLib.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

CalibrationLib calibration;

// BLE UUIDs
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Calibration parameters
int offset = 0;
float scale = 1.0;
String sensorName = "BLE Sensor";

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    
    if (value.length() > 0) {
      Serial.println("Received command: " + String(value.c_str()));
      
      // Parse command (format: "CMD:VALUE")
      String command = String(value.c_str());
      int separatorPos = command.indexOf(':');
      
      if (separatorPos > 0) {
        String cmd = command.substring(0, separatorPos);
        String val = command.substring(separatorPos + 1);
        
        if (cmd == "NAME") {
          sensorName = val;
          calibration.setCalibrationValue("name", sensorName.c_str());
          Serial.println("Name set to: " + sensorName);
        }
        else if (cmd == "OFFSET") {
          offset = val.toInt();
          calibration.setCalibrationValue("offset", offset);
          Serial.println("Offset set to: " + String(offset));
        }
        else if (cmd == "SCALE") {
          scale = val.toFloat();
          calibration.setCalibrationValue("scale", scale);
          Serial.println("Scale set to: " + String(scale, 6));
        }
        else if (cmd == "GET") {
          // Send back current values
          String response = "NAME:" + sensorName + ",OFFSET:" + String(offset) + ",SCALE:" + String(scale, 6);
          pCharacteristic->setValue(response.c_str());
          pCharacteristic->notify();
        }
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("BLE Calibration Example");
  
  // Initialize calibration
  calibration.begin("blesensor");
  
  // Load existing calibration if available
  if (calibration.hasCalibrationValue("offset") && 
      calibration.hasCalibrationValue("scale") &&
      calibration.hasCalibrationValue("name")) {
    calibration.getCalibrationValue("offset", offset);
    calibration.getCalibrationValue("scale", scale);
    calibration.getCalibrationValue("name", sensorName);
    Serial.println("Loaded existing calibration");
  } else {
    Serial.println("Using default calibration");
  }
  
  // Initialize BLE
  BLEDevice::init("ESP32 Calibration");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());
  
  pService->start();
  
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE device ready, waiting for connections...");
}

void loop() {
  // Notify value periodically if connected
  if (deviceConnected) {
    // Read sensor (simulated in this example)
    int rawValue = analogRead(36); // Replace with your actual sensor reading
    float calibratedValue = (rawValue - offset) * scale;
    
    // Format as JSON
    String json = "{\"raw\":" + String(rawValue) + ",\"cal\":" + String(calibratedValue, 2) + "}";
    
    pCharacteristic->setValue(json.c_str());
    pCharacteristic->notify();
    delay(1000); // Update once per second
  }
  
  // Disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Give the Bluetooth stack time to get ready
    pServer->startAdvertising(); // Restart advertising
    Serial.println("BLE disconnected, start advertising");
    oldDeviceConnected = deviceConnected;
  }
  
  // Connecting
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    Serial.println("BLE connected");
  }
}