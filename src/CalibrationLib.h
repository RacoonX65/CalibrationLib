#ifndef CALIBRATION_LIB_H
#define CALIBRATION_LIB_H

#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>

// Error codes
enum CalibrationError {
    CAL_OK = 0,
    CAL_NOT_INITIALIZED = -1,
    CAL_INVALID_PARAM = -2,
    CAL_WRITE_ERROR = -3,
    CAL_READ_ERROR = -4,
    CAL_MEMORY_ERROR = -5,
    CAL_ENCRYPTION_ERROR = -6
};

// Debug levels
enum DebugLevel {
    DEBUG_NONE = 0,
    DEBUG_ERROR = 1,
    DEBUG_INFO = 2,
    DEBUG_VERBOSE = 3
};

class CalibrationLib {
public:
    // Constructor
    CalibrationLib();
    
    // Debug and logging methods
    void setDebugLevel(DebugLevel level);
    void setDebugOutput(Print* output);
    CalibrationError getLastError() const;
    const char* getErrorString(CalibrationError error) const;
    
    // Validation methods
    bool validateKey(const char* key) const;
    bool validateValue(const char* key, const void* value, size_t size) const;
    
    // Batch operations
    bool batchBegin();
    bool batchCommit();
    bool batchRollback();
    
    // Encryption methods
    bool enableEncryption(const char* key);
    bool disableEncryption();
    
    // Memory management
    size_t getFreeSpace() const;
    size_t getUsedSpace() const;
    
    // Existing methods with error handling
    bool begin(const char* namespace_name = "calib");
    void end();
    
    bool setCalibrationValue(const char* key, int value);
    bool setCalibrationValue(const char* key, float value);
    bool setCalibrationValue(const char* key, const char* value);
    
    bool getCalibrationValue(const char* key, int& value, int defaultValue = 0);
    bool getCalibrationValue(const char* key, float& value, float defaultValue = 0.0f);
    bool getCalibrationValue(const char* key, String& value, const char* defaultValue = "");
    
    bool hasCalibrationValue(const char* key);
    bool removeCalibrationValue(const char* key);
    bool clearAllCalibrationValues();
    
    // JSON methods
    bool exportToJson(String& jsonString);
    bool importFromJson(const String& jsonString);
    
    // Version control
    bool setCalibrationVersion(const char* version);
    bool getCalibrationVersion(String& version);
    bool isCalibrationOutdated(const char* currentVersion);
    
    // Timestamp management
    bool setCalibrationTimestamp(unsigned long timestamp = 0);
    bool getCalibrationTimestamp(unsigned long& timestamp);
    bool isCalibrationExpired(unsigned long maxAgeMs);

// Add in private section
private:
    uint8_t _encryptionKey[32];
    Preferences _preferences;
    bool _initialized;
    DebugLevel _debugLevel;
    Print* _debugOutput;
    CalibrationError _lastError;
    bool _encryptionEnabled;
    bool _batchMode;
    
    // Internal helper methods
    void log(DebugLevel level, const char* message, ...);
    void setError(CalibrationError error);
    bool encryptData(const void* data, size_t size, uint8_t* encrypted, size_t& encSize);
    bool decryptData(const uint8_t* encrypted, size_t encSize, void* data, size_t& size);
};

#endif