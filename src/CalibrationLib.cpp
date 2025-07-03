#include "CalibrationLib.h"
#include <stdarg.h>

// Constructor with initialization
CalibrationLib::CalibrationLib() : 
    _initialized(false),
    _debugLevel(DEBUG_NONE),
    _debugOutput(&Serial),
    _lastError(CAL_OK),
    _encryptionEnabled(false),
    _batchMode(false) {
}

// Debug and logging methods
void CalibrationLib::setDebugLevel(DebugLevel level) {
    _debugLevel = level;
}

void CalibrationLib::setDebugOutput(Print* output) {
    _debugOutput = output ? output : &Serial;
}

CalibrationError CalibrationLib::getLastError() const {
    return _lastError;
}

const char* CalibrationLib::getErrorString(CalibrationError error) const {
    switch(error) {
        case CAL_OK: return "No error";
        case CAL_NOT_INITIALIZED: return "Library not initialized";
        case CAL_INVALID_PARAM: return "Invalid parameter";
        case CAL_WRITE_ERROR: return "Write error";
        case CAL_READ_ERROR: return "Read error";
        case CAL_MEMORY_ERROR: return "Memory error";
        case CAL_ENCRYPTION_ERROR: return "Encryption error";
        default: return "Unknown error";
    }
}

void CalibrationLib::log(DebugLevel level, const char* format, ...) {
    if (level <= _debugLevel && _debugOutput) {
        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        
        _debugOutput->println(buffer);
    }
}

void CalibrationLib::setError(CalibrationError error) {
    _lastError = error;
    if (error != CAL_OK) {
        log(DEBUG_ERROR, "Error: %s", getErrorString(error));
    }
}

// Validation methods
bool CalibrationLib::validateKey(const char* key) const {
    if (!key || strlen(key) == 0 || strlen(key) > 15) {
        return false;
    }
    // Check for valid characters (alphanumeric and underscore)
    for (const char* p = key; *p; p++) {
        if (!isalnum(*p) && *p != '_') {
            return false;
        }
    }
    return true;
}

bool CalibrationLib::validateValue(const char* key, const void* value, size_t size) const {
    if (!value || size == 0 || size > 4096) { // Max size for a preference value
        return false;
    }
    return true;
}

// Batch operations
bool CalibrationLib::batchBegin() {
    if (!_initialized) {
        setError(CAL_NOT_INITIALIZED);
        return false;
    }
    _batchMode = true;
    log(DEBUG_INFO, "Batch operation started");
    return true;
}

bool CalibrationLib::batchCommit() {
    if (!_initialized || !_batchMode) {
        setError(CAL_NOT_INITIALIZED);
        return false;
    }
    _batchMode = false;
    log(DEBUG_INFO, "Batch operation committed");
    return true;
}

bool CalibrationLib::batchRollback() {
    if (!_initialized || !_batchMode) {
        setError(CAL_NOT_INITIALIZED);
        return false;
    }
    _batchMode = false;
    log(DEBUG_INFO, "Batch operation rolled back");
    return true;
}

// Memory management
size_t CalibrationLib::getFreeSpace() const {
    return _initialized ? _preferences.freeEntries() : 0;
}

size_t CalibrationLib::getUsedSpace() const {
    return _initialized ? (_preferences.freeEntries() - getFreeSpace()) : 0;
}

// Modified existing methods to use new error handling
bool CalibrationLib::begin(const char* namespace_name) {
    if (!namespace_name) {
        setError(CAL_INVALID_PARAM);
        return false;
    }
    
    if (_initialized) {
        _preferences.end();
    }
    
    _initialized = _preferences.begin(namespace_name, false);
    if (!_initialized) {
        setError(CAL_NOT_INITIALIZED);
        return false;
    }
    
    log(DEBUG_INFO, "Initialized with namespace: %s", namespace_name);
    return true;
}

void CalibrationLib::end() {
  if (_initialized) {
    _preferences.end();
    _initialized = false;
  }
}

bool CalibrationLib::setCalibrationValue(const char* key, int value) {
  if (!_initialized) return false;
  return _preferences.putInt(key, value);
}

bool CalibrationLib::setCalibrationValue(const char* key, float value) {
  if (!_initialized) return false;
  return _preferences.putFloat(key, value);
}

bool CalibrationLib::setCalibrationValue(const char* key, const char* value) {
  if (!_initialized) return false;
  return _preferences.putString(key, value);
}

bool CalibrationLib::getCalibrationValue(const char* key, int& value, int defaultValue) {
  if (!_initialized) {
    value = defaultValue;
    return false;
  }
  
  value = _preferences.getInt(key, defaultValue);
  return _preferences.isKey(key);
}

bool CalibrationLib::getCalibrationValue(const char* key, float& value, float defaultValue) {
  if (!_initialized) {
    value = defaultValue;
    return false;
  }
  
  value = _preferences.getFloat(key, defaultValue);
  return _preferences.isKey(key);
}

bool CalibrationLib::getCalibrationValue(const char* key, String& value, const char* defaultValue) {
  if (!_initialized) {
    value = defaultValue;
    return false;
  }
  
  value = _preferences.getString(key, defaultValue);
  return _preferences.isKey(key);
}

bool CalibrationLib::hasCalibrationValue(const char* key) {
  if (!_initialized) return false;
  return _preferences.isKey(key);
}

bool CalibrationLib::removeCalibrationValue(const char* key) {
  if (!_initialized) return false;
  return _preferences.remove(key);
}

bool CalibrationLib::clearAllCalibrationValues() {
  if (!_initialized) return false;
  return _preferences.clear();
}

bool CalibrationLib::exportToJson(String& jsonString) {
  if (!_initialized) return false;
  
  StaticJsonDocument<512> doc;
  JsonObject root = doc.to<JsonObject>();
  
  // Get all keys and their values
  for (size_t i = 0; i < _preferences.freeEntries(); i++) {
    String key = _preferences.key(i);
    String type = _preferences.getType(key.c_str());
    
    if (type == "i") {
      root[key] = _preferences.getInt(key.c_str());
    } else if (type == "f") {
      root[key] = _preferences.getFloat(key.c_str());
    } else if (type == "s") {
      root[key] = _preferences.getString(key.c_str());
    }
  }
  
  serializeJson(doc, jsonString);
  return true;
}

bool CalibrationLib::importFromJson(const String& jsonString) {
  if (!_initialized) return false;
  
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, jsonString);
  
  if (error) return false;
  
  JsonObject root = doc.as<JsonObject>();
  for (JsonPair kv : root) {
    if (kv.value().is<int>()) {
      setCalibrationValue(kv.key().c_str(), kv.value().as<int>());
    } else if (kv.value().is<float>()) {
      setCalibrationValue(kv.key().c_str(), kv.value().as<float>());
    } else if (kv.value().is<const char*>()) {
      setCalibrationValue(kv.key().c_str(), kv.value().as<const char*>());
    }
  }
  
  return true;
}

bool CalibrationLib::setCalibrationVersion(const char* version) {
  if (!_initialized) return false;
  return _preferences.putString("_version", version);
}

bool CalibrationLib::getCalibrationVersion(String& version) {
  if (!_initialized) {
    version = "";
    return false;
  }
  version = _preferences.getString("_version", "");
  return _preferences.isKey("_version");
}

bool CalibrationLib::isCalibrationOutdated(const char* currentVersion) {
  String storedVersion;
  if (!getCalibrationVersion(storedVersion)) return true;
  return storedVersion != currentVersion;
}

bool CalibrationLib::setCalibrationTimestamp(unsigned long timestamp) {
  if (!_initialized) return false;
  if (timestamp == 0) timestamp = millis();
  return _preferences.putULong("_timestamp", timestamp);
}

bool CalibrationLib::getCalibrationTimestamp(unsigned long& timestamp) {
  if (!_initialized) {
    timestamp = 0;
    return false;
  }
  timestamp = _preferences.getULong("_timestamp", 0);
  return _preferences.isKey("_timestamp");
}

bool CalibrationLib::isCalibrationExpired(unsigned long maxAgeMs) {
  unsigned long timestamp;
  if (!getCalibrationTimestamp(timestamp)) return true;
  return (millis() - timestamp) > maxAgeMs;
}

// Add these includes at the top
#include <mbedtls/aes.h>
#include <mbedtls/md.h>

bool CalibrationLib::enableEncryption(const char* key) {
    if (!key || strlen(key) < 16) {
        setError(CAL_ENCRYPTION_ERROR);
        return false;
    }
    
    // Generate encryption key using SHA256
    uint8_t derivedKey[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char*)key, strlen(key));
    mbedtls_md_finish(&ctx, derivedKey);
    mbedtls_md_free(&ctx);
    
    // Store the key securely
    memcpy(_encryptionKey, derivedKey, 32);
    _encryptionEnabled = true;
    
    log(DEBUG_INFO, "Encryption enabled");
    return true;
}

bool CalibrationLib::disableEncryption() {
    if (!_encryptionEnabled) {
        return true;
    }
    
    // Clear encryption key
    memset(_encryptionKey, 0, sizeof(_encryptionKey));
    _encryptionEnabled = false;
    
    log(DEBUG_INFO, "Encryption disabled");
    return true;
}

bool CalibrationLib::encryptData(const void* data, size_t size, uint8_t* encrypted, size_t& encSize) {
    if (!_encryptionEnabled || !data || !encrypted) {
        setError(CAL_ENCRYPTION_ERROR);
        return false;
    }
    
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, _encryptionKey, 256);
    
    // Add PKCS7 padding
    uint8_t paddingSize = 16 - (size % 16);
    size_t totalSize = size + paddingSize;
    uint8_t* paddedData = new uint8_t[totalSize];
    memcpy(paddedData, data, size);
    memset(paddedData + size, paddingSize, paddingSize);
    
    // Encrypt
    for (size_t i = 0; i < totalSize; i += 16) {
        mbedtls_aes_encrypt(&aes, paddedData + i, encrypted + i);
    }
    
    encSize = totalSize;
    delete[] paddedData;
    mbedtls_aes_free(&aes);
    
    return true;
}

bool CalibrationLib::decryptData(const uint8_t* encrypted, size_t encSize, void* data, size_t& size) {
    if (!_encryptionEnabled || !encrypted || !data || encSize % 16 != 0) {
        setError(CAL_ENCRYPTION_ERROR);
        return false;
    }
    
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, _encryptionKey, 256);
    
    uint8_t* decrypted = new uint8_t[encSize];
    
    // Decrypt
    for (size_t i = 0; i < encSize; i += 16) {
        mbedtls_aes_decrypt(&aes, encrypted + i, decrypted + i);
    }
    
    // Remove PKCS7 padding
    uint8_t paddingSize = decrypted[encSize - 1];
    if (paddingSize > 16) {
        delete[] decrypted;
        mbedtls_aes_free(&aes);
        setError(CAL_ENCRYPTION_ERROR);
        return false;
    }
    
    size = encSize - paddingSize;
    memcpy(data, decrypted, size);
    
    delete[] decrypted;
    mbedtls_aes_free(&aes);
    
    return true;
}