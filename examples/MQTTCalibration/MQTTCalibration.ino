/*
  MQTTCalibration.ino
  Example for CalibrationLib: MQTT Configuration Management

  This example demonstrates how to use CalibrationLib to manage MQTT broker
  configuration settings. It shows how to:
  - Store and retrieve MQTT connection parameters
  - Update configuration via MQTT messages
  - Maintain persistent broker settings
  - Handle connection management

  Features:
  - Remote MQTT configuration updates
  - Persistent configuration storage
  - JSON-based configuration messages
  - Automatic reconnection handling
  - Default fallback values
  - Configuration status reporting

  MQTT Topics:
  - device/calibration/set: Receive configuration updates
  - device/calibration/status: Report current configuration

  Configuration Parameters:
  - mqtt_server: Broker hostname (default: broker.hivemq.com)
  - mqtt_port: Broker port (default: 1883)
  - mqtt_user: Username for authentication
  - mqtt_pass: Password for authentication

  JSON Message Format:
  {
    "mqtt_server": "broker.example.com",
    "mqtt_port": 1883,
    "mqtt_user": "username",
    "mqtt_pass": "password"
  }

  Hardware: ESP32 development board with WiFi capability
  Software: Arduino IDE with:
  - ESP32 Arduino Core
  - PubSubClient library
  - ArduinoJson library
  - CalibrationLib

  Note: Update WiFi credentials before uploading.

  Author: Judas Sithole (judassithole@duck.com)
  Created: 2025
  License: MIT
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <CalibrationLib.h>
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient mqtt(espClient);
CalibrationLib calib;

// Calibration keys
const char* MQTT_SERVER_KEY = "mqtt_server";
const char* MQTT_PORT_KEY = "mqtt_port";
const char* MQTT_USER_KEY = "mqtt_user";
const char* MQTT_PASS_KEY = "mqtt_pass";

void setup() {
  Serial.begin(115200);
  
  // Initialize calibration
  calib.begin("mqtt_config");
  
  // Connect to WiFi (assuming WiFi credentials are set)
  WiFi.begin("your_ssid", "your_password");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Load MQTT settings from calibration
  String mqttServer, mqttUser, mqttPass;
  int mqttPort;
  
  calib.getCalibrationValue(MQTT_SERVER_KEY, mqttServer, "broker.hivemq.com");
  calib.getCalibrationValue(MQTT_PORT_KEY, mqttPort, 1883);
  calib.getCalibrationValue(MQTT_USER_KEY, mqttUser, "");
  calib.getCalibrationValue(MQTT_PASS_KEY, mqttPass, "");
  
  // Configure MQTT
  mqtt.setServer(mqttServer.c_str(), mqttPort);
  mqtt.setCallback(mqttCallback);
  
  // Connect to MQTT
  reconnectMQTT(mqttUser.c_str(), mqttPass.c_str());
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming calibration commands
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload, length);
  
  if (strcmp(topic, "device/calibration/set") == 0) {
    if (doc.containsKey("mqtt_server")) {
      calib.setCalibrationValue(MQTT_SERVER_KEY, doc["mqtt_server"].as<const char*>());
    }
    if (doc.containsKey("mqtt_port")) {
      calib.setCalibrationValue(MQTT_PORT_KEY, doc["mqtt_port"].as<int>());
    }
    if (doc.containsKey("mqtt_user")) {
      calib.setCalibrationValue(MQTT_USER_KEY, doc["mqtt_user"].as<const char*>());
    }
    if (doc.containsKey("mqtt_pass")) {
      calib.setCalibrationValue(MQTT_PASS_KEY, doc["mqtt_pass"].as<const char*>());
    }
    
    // Publish current configuration
    publishCurrentConfig();
  }
}

void publishCurrentConfig() {
  StaticJsonDocument<200> doc;
  String mqttServer, mqttUser;
  int mqttPort;
  
  calib.getCalibrationValue(MQTT_SERVER_KEY, mqttServer, "broker.hivemq.com");
  calib.getCalibrationValue(MQTT_PORT_KEY, mqttPort, 1883);
  calib.getCalibrationValue(MQTT_USER_KEY, mqttUser, "");
  
  doc["mqtt_server"] = mqttServer;
  doc["mqtt_port"] = mqttPort;
  doc["mqtt_user"] = mqttUser;
  
  String output;
  serializeJson(doc, output);
  mqtt.publish("device/calibration/status", output.c_str());
}

void reconnectMQTT(const char* user, const char* pass) {
  while (!mqtt.connected()) {
    Serial.println("Connecting to MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    
    if (mqtt.connect(clientId.c_str(), user, pass)) {
      Serial.println("Connected");
      mqtt.subscribe("device/calibration/set");
      publishCurrentConfig();
    } else {
      Serial.printf("Failed, rc=%d\n", mqtt.state());
      delay(5000);
    }
  }
}

void loop() {
  if (!mqtt.connected()) {
    String mqttUser, mqttPass;
    calib.getCalibrationValue(MQTT_USER_KEY, mqttUser, "");
    calib.getCalibrationValue(MQTT_PASS_KEY, mqttPass, "");
    reconnectMQTT(mqttUser.c_str(), mqttPass.c_str());
  }
  mqtt.loop();
}