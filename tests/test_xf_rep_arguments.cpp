/**
 * @file test_xf_rep_arguments.cpp
 * @brief Unit test for XF_REP_ARGUMENTS handler
 * 
 * This test verifies that the XF_REP_ARGUMENTS handler correctly:
 * 1. Loads the JSON configuration file
 * 2. Validates the configuration
 * 3. Returns the correct input and output counts
 * 4. Handles missing or invalid configuration files
 * 
 * Requirements: 1.5, 7.4
 */

#include <iostream>
#include <fstream>
#include <cstring>
#include <Windows.h>

// Forward declaration of the EpanetBridge function
extern "C" void __declspec(dllimport) EpanetBridge(int methodID, int* status, double* inargs, double* outargs);

// XFMethod enumeration
enum XFMethodID {
    XF_INITIALIZE = 0,
    XF_CALCULATE = 1,
    XF_REP_VERSION = 2,
    XF_REP_ARGUMENTS = 3,
    XF_CLEANUP = 99
};

// XFStatus enumeration
enum XFStatusID {
    XF_SUCCESS = 0,
    XF_FAILURE = 1,
    XF_CLEANUP_NOW = 99,
    XF_FAILURE_WITH_MSG = -1,
    XF_INCREASE_MEMORY = -2
};

/**
 * @brief Test XF_REP_ARGUMENTS with valid configuration
 */
bool test_valid_config() {
    std::cout << "Test: XF_REP_ARGUMENTS with valid configuration..." << std::endl;
    
    // Copy test configuration to working directory
    std::ifstream src("test_data/valid_config.json", std::ios::binary);
    std::ofstream dst("EpanetBridge.json", std::ios::binary);
    dst << src.rdbuf();
    src.close();
    dst.close();
    
    // Call XF_REP_ARGUMENTS
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    EpanetBridge(XF_REP_ARGUMENTS, &status, inargs, outargs);
    
    // Verify status is success
    if (status != XF_SUCCESS) {
        std::cout << "  FAILED: Expected status XF_SUCCESS (0), got " << status << std::endl;
        if (status == XF_FAILURE_WITH_MSG) {
            ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
            char* errorMsg = (char*)(*pAddr);
            std::cout << "  Error message: " << errorMsg << std::endl;
        }
        return false;
    }
    
    // Verify input count (should be 3)
    if (outargs[0] != 3.0) {
        std::cout << "  FAILED: Expected input_count = 3, got " << outargs[0] << std::endl;
        return false;
    }
    
    // Verify output count (should be 4)
    if (outargs[1] != 4.0) {
        std::cout << "  FAILED: Expected output_count = 4, got " << outargs[1] << std::endl;
        return false;
    }
    
    std::cout << "  PASSED: input_count = " << outargs[0] << ", output_count = " << outargs[1] << std::endl;
    return true;
}

/**
 * @brief Test XF_REP_ARGUMENTS with missing configuration file
 */
bool test_missing_config() {
    std::cout << "Test: XF_REP_ARGUMENTS with missing configuration..." << std::endl;
    
    // Remove configuration file if it exists
    remove("EpanetBridge.json");
    
    // Call XF_REP_ARGUMENTS
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    EpanetBridge(XF_REP_ARGUMENTS, &status, inargs, outargs);
    
    // Verify status is failure with message
    if (status != XF_FAILURE_WITH_MSG) {
        std::cout << "  FAILED: Expected status XF_FAILURE_WITH_MSG (-1), got " << status << std::endl;
        return false;
    }
    
    // Verify error message is present
    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
    char* errorMsg = (char*)(*pAddr);
    
    if (errorMsg == nullptr || strlen(errorMsg) == 0) {
        std::cout << "  FAILED: Expected error message, got nullptr or empty string" << std::endl;
        return false;
    }
    
    std::cout << "  PASSED: Error message = \"" << errorMsg << "\"" << std::endl;
    return true;
}

/**
 * @brief Test XF_REP_ARGUMENTS with invalid JSON (missing required field)
 */
bool test_invalid_config() {
    std::cout << "Test: XF_REP_ARGUMENTS with invalid configuration..." << std::endl;
    
    // Create invalid configuration (missing input_count)
    std::ofstream file("EpanetBridge.json");
    file << "{\n";
    file << "  \"version\": \"1.0\",\n";
    file << "  \"output_count\": 4,\n";
    file << "  \"inputs\": [],\n";
    file << "  \"outputs\": []\n";
    file << "}\n";
    file.close();
    
    // Call XF_REP_ARGUMENTS
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    EpanetBridge(XF_REP_ARGUMENTS, &status, inargs, outargs);
    
    // Verify status is failure with message
    if (status != XF_FAILURE_WITH_MSG) {
        std::cout << "  FAILED: Expected status XF_FAILURE_WITH_MSG (-1), got " << status << std::endl;
        return false;
    }
    
    // Verify error message is present
    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
    char* errorMsg = (char*)(*pAddr);
    
    if (errorMsg == nullptr || strlen(errorMsg) == 0) {
        std::cout << "  FAILED: Expected error message, got nullptr or empty string" << std::endl;
        return false;
    }
    
    std::cout << "  PASSED: Error message = \"" << errorMsg << "\"" << std::endl;
    return true;
}

/**
 * @brief Test XF_REP_ARGUMENTS with count mismatch
 */
bool test_count_mismatch() {
    std::cout << "Test: XF_REP_ARGUMENTS with count mismatch..." << std::endl;
    
    // Create configuration with count mismatch
    std::ofstream file("EpanetBridge.json");
    file << "{\n";
    file << "  \"version\": \"1.0\",\n";
    file << "  \"input_count\": 5,\n";
    file << "  \"output_count\": 4,\n";
    file << "  \"inputs\": [\n";
    file << "    {\"index\": 0, \"name\": \"ElapsedTime\", \"object_type\": \"SYSTEM\", \"property\": \"ELAPSEDTIME\"}\n";
    file << "  ],\n";
    file << "  \"outputs\": []\n";
    file << "}\n";
    file.close();
    
    // Call XF_REP_ARGUMENTS
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    EpanetBridge(XF_REP_ARGUMENTS, &status, inargs, outargs);
    
    // Verify status is failure with message
    if (status != XF_FAILURE_WITH_MSG) {
        std::cout << "  FAILED: Expected status XF_FAILURE_WITH_MSG (-1), got " << status << std::endl;
        return false;
    }
    
    // Verify error message contains "mismatch"
    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
    char* errorMsg = (char*)(*pAddr);
    
    if (errorMsg == nullptr || strstr(errorMsg, "mismatch") == nullptr) {
        std::cout << "  FAILED: Expected error message to contain 'mismatch'" << std::endl;
        if (errorMsg) {
            std::cout << "  Got: \"" << errorMsg << "\"" << std::endl;
        }
        return false;
    }
    
    std::cout << "  PASSED: Error message = \"" << errorMsg << "\"" << std::endl;
    return true;
}

/**
 * @brief Main test runner
 */
int main() {
    std::cout << "=== XF_REP_ARGUMENTS Unit Tests ===" << std::endl;
    std::cout << std::endl;
    
    int passed = 0;
    int failed = 0;
    
    // Run tests
    if (test_valid_config()) passed++; else failed++;
    std::cout << std::endl;
    
    if (test_missing_config()) passed++; else failed++;
    std::cout << std::endl;
    
    if (test_invalid_config()) passed++; else failed++;
    std::cout << std::endl;
    
    if (test_count_mismatch()) passed++; else failed++;
    std::cout << std::endl;
    
    // Print summary
    std::cout << "=== Test Summary ===" << std::endl;
    std::cout << "Passed: " << passed << std::endl;
    std::cout << "Failed: " << failed << std::endl;
    std::cout << "Total:  " << (passed + failed) << std::endl;
    
    // Cleanup
    remove("EpanetBridge.json");
    
    return (failed == 0) ? 0 : 1;
}
