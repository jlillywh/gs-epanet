/**
 * @brief Standalone test to verify Logger functionality
 * 
 * This is a simple standalone test that can be compiled and run
 * independently to verify the Logger class works correctly.
 */

#include "../src/Logger.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

bool TestLogger() {
    const std::string testLogFile = "test_standalone_logger.log";
    
    // Clean up any existing test log file
    if (fs::exists(testLogFile)) {
        fs::remove(testLogFile);
    }
    
    std::cout << "Testing Logger class..." << std::endl;
    
    // Test 1: Create logger and set log file
    Logger logger;
    if (!logger.SetLogFile(testLogFile)) {
        std::cerr << "FAILED: Could not set log file" << std::endl;
        return false;
    }
    std::cout << "PASSED: SetLogFile()" << std::endl;
    
    // Test 2: Test OFF level (no messages should be written)
    logger.SetLevel(LogLevel::OFF);
    logger.Error("This should not appear");
    logger.Info("This should not appear");
    logger.Debug("This should not appear");
    logger.Close();
    
    std::ifstream checkFile(testLogFile);
    std::string line;
    int lineCount = 0;
    while (std::getline(checkFile, line)) {
        if (!line.empty()) lineCount++;
    }
    checkFile.close();
    
    if (lineCount != 0) {
        std::cerr << "FAILED: OFF level wrote " << lineCount << " lines (expected 0)" << std::endl;
        return false;
    }
    std::cout << "PASSED: LogLevel::OFF filtering" << std::endl;
    
    // Test 3: Test ERROR level
    logger.SetLogFile(testLogFile);
    logger.SetLevel(LogLevel::ERROR);
    logger.Error("Error message");
    logger.Info("Info message");
    logger.Debug("Debug message");
    logger.Close();
    
    std::ifstream errorFile(testLogFile);
    std::string content;
    while (std::getline(errorFile, line)) {
        content += line + "\n";
    }
    errorFile.close();
    
    if (content.find("Error message") == std::string::npos) {
        std::cerr << "FAILED: ERROR level did not write error message" << std::endl;
        return false;
    }
    if (content.find("Info message") != std::string::npos) {
        std::cerr << "FAILED: ERROR level wrote info message" << std::endl;
        return false;
    }
    if (content.find("Debug message") != std::string::npos) {
        std::cerr << "FAILED: ERROR level wrote debug message" << std::endl;
        return false;
    }
    std::cout << "PASSED: LogLevel::ERROR filtering" << std::endl;
    
    // Test 4: Test INFO level
    fs::remove(testLogFile);
    logger.SetLogFile(testLogFile);
    logger.SetLevel(LogLevel::INFO);
    logger.Error("Error message");
    logger.Info("Info message");
    logger.Debug("Debug message");
    logger.Close();
    
    std::ifstream infoFile(testLogFile);
    content.clear();
    while (std::getline(infoFile, line)) {
        content += line + "\n";
    }
    infoFile.close();
    
    if (content.find("Error message") == std::string::npos) {
        std::cerr << "FAILED: INFO level did not write error message" << std::endl;
        return false;
    }
    if (content.find("Info message") == std::string::npos) {
        std::cerr << "FAILED: INFO level did not write info message" << std::endl;
        return false;
    }
    if (content.find("Debug message") != std::string::npos) {
        std::cerr << "FAILED: INFO level wrote debug message" << std::endl;
        return false;
    }
    std::cout << "PASSED: LogLevel::INFO filtering" << std::endl;
    
    // Test 5: Test DEBUG level
    fs::remove(testLogFile);
    logger.SetLogFile(testLogFile);
    logger.SetLevel(LogLevel::DEBUG);
    logger.Error("Error message");
    logger.Info("Info message");
    logger.Debug("Debug message");
    logger.Close();
    
    std::ifstream debugFile(testLogFile);
    content.clear();
    while (std::getline(debugFile, line)) {
        content += line + "\n";
    }
    debugFile.close();
    
    if (content.find("Error message") == std::string::npos) {
        std::cerr << "FAILED: DEBUG level did not write error message" << std::endl;
        return false;
    }
    if (content.find("Info message") == std::string::npos) {
        std::cerr << "FAILED: DEBUG level did not write info message" << std::endl;
        return false;
    }
    if (content.find("Debug message") == std::string::npos) {
        std::cerr << "FAILED: DEBUG level did not write debug message" << std::endl;
        return false;
    }
    std::cout << "PASSED: LogLevel::DEBUG filtering" << std::endl;
    
    // Test 6: Test timestamp format (ISO 8601)
    if (content.find("[") == std::string::npos || 
        content.find("T") == std::string::npos ||
        content.find("]") == std::string::npos) {
        std::cerr << "FAILED: Timestamp format incorrect" << std::endl;
        return false;
    }
    
    // Extract timestamp
    size_t start = content.find("[");
    size_t end = content.find("]");
    std::string timestamp = content.substr(start + 1, end - start - 1);
    
    // Check ISO 8601 format: YYYY-MM-DDTHH:MM:SS.mmm
    if (timestamp.length() != 23 ||
        timestamp[4] != '-' ||
        timestamp[7] != '-' ||
        timestamp[10] != 'T' ||
        timestamp[13] != ':' ||
        timestamp[16] != ':' ||
        timestamp[19] != '.') {
        std::cerr << "FAILED: Timestamp not in ISO 8601 format: " << timestamp << std::endl;
        return false;
    }
    std::cout << "PASSED: ISO 8601 timestamp format" << std::endl;
    
    // Test 7: Test log level labels
    if (content.find("[ERROR]") == std::string::npos) {
        std::cerr << "FAILED: Missing [ERROR] label" << std::endl;
        return false;
    }
    if (content.find("[INFO]") == std::string::npos) {
        std::cerr << "FAILED: Missing [INFO] label" << std::endl;
        return false;
    }
    if (content.find("[DEBUG]") == std::string::npos) {
        std::cerr << "FAILED: Missing [DEBUG] label" << std::endl;
        return false;
    }
    std::cout << "PASSED: Log level labels" << std::endl;
    
    // Clean up
    if (fs::exists(testLogFile)) {
        fs::remove(testLogFile);
    }
    
    std::cout << "\nAll Logger tests PASSED!" << std::endl;
    return true;
}

int main() {
    if (TestLogger()) {
        return 0;
    } else {
        return 1;
    }
}
