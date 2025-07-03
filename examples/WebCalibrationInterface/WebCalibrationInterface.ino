/*
  WebCalibrationInterface.ino
  Example for CalibrationLib: Web-Based Calibration Interface

  This example creates a responsive web interface for sensor
  calibration, allowing users to view and modify calibration
  parameters through a browser without needing to modify code.

  Features:
  - Browser-based calibration interface
  - Real-time sensor data display
  - Responsive design for mobile/desktop
  - Persistent calibration storage
  - JSON-based data updates
  - Error handling and validation

  Calibration Parameters:
  1. Sensor Name (string)
     - Identifies the sensor
     - Default: "Default Sensor"

  2. Offset (int)
     - Zero-point correction
     - Default: 0

  3. Scale Factor (float)
     - Sensitivity adjustment
     - Default: 1.0

  Web Interface:
  - URL: http://<ESP32_IP>
  - Responsive layout
  - Live data updates (1 second interval)
  - Form validation
  - Status feedback

  Network Setup:
  - Connects to existing WiFi network
  - Creates HTTP server on port 80
  - Displays IP address via Serial

  API Endpoints:
  - GET  /      : Main interface
  - POST /save  : Save calibration
  - GET  /data  : Live sensor data

  Data Format:
  - Calibration: Form data
  - Live Data: JSON {"raw":value,"calibrated":value}

  Hardware Setup:
  - ESP32 development board
  - WiFi connection
  - Analog sensor on GPIO36 (optional)
  - Serial connection (115200 baud)

  Storage:
  - Namespace: "websensor"
  - Keys: "name", "offset", "scale"

  Dependencies:
  - ESP32 Arduino Core
  - WiFi library
  - WebServer library
  - CalibrationLib

  Note: Replace WiFi credentials (ssid/password) with your
  network details before uploading.

  Author: Judas Sithole (judassithole@duck.com)
  Created: 2025
  License: MIT
*/

#include <CalibrationLib.h>
#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

CalibrationLib calibration;
WebServer server(80);

// Calibration parameters
int offset = 0;
float scale = 1.0;
String sensorName = "Default Sensor";

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize calibration
  calibration.begin("websensor");
  
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
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  
  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/data", handleData);
  server.onNotFound(handleNotFound);
  
  // Start server
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("Open a browser and navigate to http://" + WiFi.localIP().toString());
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = "<!DOCTYPE html>\n"
               "<html>\n"
               "<head>\n"
               "  <title>ESP32 Calibration Interface</title>\n"
               "  <meta name='viewport' content='width=device-width, initial-scale=1'>\n"
               "  <style>\n"
               "    body { font-family: Arial; margin: 20px; }\n"
               "    .container { max-width: 500px; margin: 0 auto; }\n"
               "    h1 { color: #0066cc; }\n"
               "    label { display: block; margin-top: 10px; }\n"
               "    input { width: 100%; padding: 5px; margin-top: 5px; }\n"
               "    button { background-color: #0066cc; color: white; border: none; padding: 10px 15px; margin-top: 20px; cursor: pointer; }\n"
               "    .status { margin-top: 20px; padding: 10px; background-color: #f0f0f0; }\n"
               "  </style>\n"
               "</head>\n"
               "<body>\n"
               "  <div class='container'>\n"
               "    <h1>ESP32 Sensor Calibration</h1>\n"
               "    <form id='calibrationForm'>\n"
               "      <label for='name'>Sensor Name:</label>\n"
               "      <input type='text' id='name' name='name' value='" + sensorName + "'>\n"
               "      <label for='offset'>Offset:</label>\n"
               "      <input type='number' id='offset' name='offset' value='" + String(offset) + "'>\n"
               "      <label for='scale'>Scale Factor:</label>\n"
               "      <input type='number' id='scale' name='scale' step='0.0001' value='" + String(scale) + "'>\n"
               "      <button type='button' onclick='saveCalibration()'>Save Calibration</button>\n"
               "    </form>\n"
               "    <div id='status' class='status'>Ready</div>\n"
               "    <div class='status'>\n"
               "      <h3>Live Data</h3>\n"
               "      <p>Raw Value: <span id='rawValue'>-</span></p>\n"
               "      <p>Calibrated Value: <span id='calibratedValue'>-</span></p>\n"
               "    </div>\n"
               "  </div>\n"
               "  <script>\n"
               "    function saveCalibration() {\n"
               "      const form = document.getElementById('calibrationForm');\n"
               "      const formData = new FormData(form);\n"
               "      fetch('/save', {\n"
               "        method: 'POST',\n"
               "        body: formData\n"
               "      })\n"
               "      .then(response => response.text())\n"
               "      .then(data => {\n"
               "        document.getElementById('status').innerText = data;\n"
               "      });\n"
               "    }\n"
               "    // Update live data every second\n"
               "    setInterval(() => {\n"
               "      fetch('/data')\n"
               "      .then(response => response.json())\n"
               "      .then(data => {\n"
               "        document.getElementById('rawValue').innerText = data.raw;\n"
               "        document.getElementById('calibratedValue').innerText = data.calibrated;\n"
               "      });\n"
               "    }, 1000);\n"
               "  </script>\n"
               "</body>\n"
               "</html>";
  
  server.send(200, "text/html", html);
}

void handleSave() {
  if (server.hasArg("name") && server.hasArg("offset") && server.hasArg("scale")) {
    sensorName = server.arg("name");
    offset = server.arg("offset").toInt();
    scale = server.arg("scale").toFloat();
    
    // Save to flash memory
    calibration.setCalibrationValue("name", sensorName.c_str());
    calibration.setCalibrationValue("offset", offset);
    calibration.setCalibrationValue("scale", scale);
    
    server.send(200, "text/plain", "Calibration saved successfully!");
  } else {
    server.send(400, "text/plain", "Missing parameters!");
  }
}

void handleData() {
  // Read sensor (simulated in this example)
  int rawValue = analogRead(36); // Replace with your actual sensor reading
  float calibratedValue = (rawValue - offset) * scale;
  
  String json = "{\"raw\":" + String(rawValue) + ",\"calibrated\":" + String(calibratedValue, 2) + "}";
  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}