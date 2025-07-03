/*
  UnitTests.ino
  Example for CalibrationLib: Comprehensive Unit Testing Suite

  This example demonstrates how to use Unity testing framework to verify
  the functionality of CalibrationLib. It includes a comprehensive test
  suite that validates:
  - Basic library operations
  - Data type handling
  - Encryption features
  - JSON import/export

  Features Tested:
  - Library initialization
  - Integer value storage/retrieval
  - Float value storage/retrieval
  - String value storage/retrieval
  - Encryption enable/disable
  - Encrypted data storage
  - JSON data export
  - JSON data import
  - Error handling
  - Memory cleanup

  Test Categories:
  1. Initialization Tests
     - Library initialization
     - Error state verification

  2. Basic Operations Tests
     - Integer operations (42)
     - Float operations (3.14)
     - String operations ("Hello")

  3. Encryption Tests
     - Key management
     - Data encryption
     - Data decryption

  4. JSON Operations Tests
     - Multi-type data export
     - Data clearing
     - Data import
     - Value verification

  Test Environment:
  - ESP32 development board
  - Serial connection for test results (default baud)
  - Unity test framework

  Test Execution:
  - Tests run automatically on startup
  - Results output to Serial monitor
  - Each test category runs sequentially
  - Automatic cleanup between tests

  Dependencies:
  - ESP32 Arduino Core
  - Unity Test Framework
  - CalibrationLib

  Note: This test suite is designed for development and
  verification purposes. It helps ensure the library
  functions correctly across updates and modifications.

  Author: Judas Sithole (judassithole@duck.com)
  Created: 2025
  License: MIT
*/

#include <CalibrationLib.h>
#include <unity.h>

CalibrationLib calibration;

void setUp(void) {
    calibration.begin("test");
}

void tearDown(void) {
    calibration.clearAllCalibrationValues();
    calibration.end();
}

void test_initialization(void) {
    TEST_ASSERT_TRUE(calibration.begin("test"));
    TEST_ASSERT_EQUAL(CAL_OK, calibration.getLastError());
}

void test_basic_operations(void) {
    // Integer tests
    TEST_ASSERT_TRUE(calibration.setCalibrationValue("int_test", 42));
    int intValue;
    TEST_ASSERT_TRUE(calibration.getCalibrationValue("int_test", intValue));
    TEST_ASSERT_EQUAL(42, intValue);
    
    // Float tests
    TEST_ASSERT_TRUE(calibration.setCalibrationValue("float_test", 3.14f));
    float floatValue;
    TEST_ASSERT_TRUE(calibration.getCalibrationValue("float_test", floatValue));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 3.14f, floatValue);
    
    // String tests
    TEST_ASSERT_TRUE(calibration.setCalibrationValue("str_test", "Hello"));
    String strValue;
    TEST_ASSERT_TRUE(calibration.getCalibrationValue("str_test", strValue));
    TEST_ASSERT_EQUAL_STRING("Hello", strValue.c_str());
}

void test_encryption(void) {
    TEST_ASSERT_TRUE(calibration.enableEncryption("MySecretKey12345"));
    TEST_ASSERT_TRUE(calibration.setCalibrationValue("encrypted_test", "SecretData"));
    String value;
    TEST_ASSERT_TRUE(calibration.getCalibrationValue("encrypted_test", value));
    TEST_ASSERT_EQUAL_STRING("SecretData", value.c_str());
    TEST_ASSERT_TRUE(calibration.disableEncryption());
}

void test_json_operations(void) {
    calibration.setCalibrationValue("test_int", 42);
    calibration.setCalibrationValue("test_float", 3.14f);
    calibration.setCalibrationValue("test_string", "Hello");
    
    String jsonData;
    TEST_ASSERT_TRUE(calibration.exportToJson(jsonData));
    
    calibration.clearAllCalibrationValues();
    TEST_ASSERT_TRUE(calibration.importFromJson(jsonData));
    
    int intVal;
    float floatVal;
    String strVal;
    calibration.getCalibrationValue("test_int", intVal);
    calibration.getCalibrationValue("test_float", floatVal);
    calibration.getCalibrationValue("test_string", strVal);
    
    TEST_ASSERT_EQUAL(42, intVal);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 3.14f, floatVal);
    TEST_ASSERT_EQUAL_STRING("Hello", strVal.c_str());
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_initialization);
    RUN_TEST(test_basic_operations);
    RUN_TEST(test_encryption);
    RUN_TEST(test_json_operations);
    UNITY_END();
}

void loop() {}