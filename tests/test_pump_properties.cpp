/**
 * @file test_pump_properties.cpp
 * @brief Test POWER and EFFICIENCY output properties for pumps
 * 
 * This test validates that the bridge correctly retrieves pump power
 * and efficiency values from EPANET using the EN_PUMP_POWER and
 * EN_PUMP_EFFIC property codes.
 * 
 * Requirements: 5.1-5.8 (Output extraction)
 */

#include <iostream>
#include <iomanip>
#include <cmath>
#include "../src/EpanetBridge.h"

// Test result tracking
int g_testsPassed = 0;
int g_testsFailed = 0;

/**
 * @brief Assert helper function
 */
void Assert(bool condition, const char* message) {
    if (condition) {
        std::cout << "[PASS] " << message << std::endl;
        g_testsPassed++;
    } else {
        std::cout << "[FAIL] " << message << std::endl;
        g_testsFailed++;
    }
}

/**
 * @brief Test pump POWER and EFFICIENCY properties
 * 
 * This test:
 * 1. Initializes the bridge with Pump_to_Tank model
 * 2. Runs a calculation with pump on (setting = 1.0)
 * 3. Verifies POWER output is non-negative
 * 4. Verifies EFFICIENCY output is in valid range (0-100%)
 * 5. Runs calculation with pump off (setting = 0.0)
 * 6. Verifies EFFICIENCY is 0 when pump is off
 */
int main() {
    std::cout << "=== Test Pump POWER and EFFICIENCY Properties ===" << std::endl;
    std::cout << std::endl;
    
    int status = 0;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Test 1: XF_REP_VERSION
    std::cout << "Test 1: XF_REP_VERSION" << std::endl;
    EpanetBridge(2, &status, inargs, outargs);
    Assert(status == 0, "XF_REP_VERSION returns success");
    Assert(outargs[0] == 1.1, "Version is 1.1");
    std::cout << "  Version: " << outargs[0] << std::endl;
    std::cout << std::endl;
    
    // Test 2: XF_REP_ARGUMENTS
    std::cout << "Test 2: XF_REP_ARGUMENTS" << std::endl;
    EpanetBridge(3, &status, inargs, outargs);
    Assert(status == 0, "XF_REP_ARGUMENTS returns success");
    int inputCount = (int)outargs[0];
    int outputCount = (int)outargs[1];
    std::cout << "  Input count: " << inputCount << std::endl;
    std::cout << "  Output count: " << outputCount << std::endl;
    Assert(inputCount >= 2, "At least 2 inputs (ElapsedTime + pump setting)");
    Assert(outputCount >= 2, "At least 2 outputs (POWER + EFFICIENCY)");
    std::cout << std::endl;
    
    // Test 3: XF_INITIALIZE
    std::cout << "Test 3: XF_INITIALIZE" << std::endl;
    EpanetBridge(0, &status, inargs, outargs);
    Assert(status == 0, "XF_INITIALIZE returns success");
    std::cout << std::endl;
    
    // Test 4: XF_CALCULATE with pump ON (setting = 1.0)
    std::cout << "Test 4: XF_CALCULATE with pump ON" << std::endl;
    inargs[0] = 300.0;  // ElapsedTime = 300 seconds (5 minutes)
    inargs[1] = 1.0;    // Pump setting = 1.0 (full speed, ON)
    
    EpanetBridge(1, &status, inargs, outargs);
    Assert(status == 0, "XF_CALCULATE returns success");
    
    double tankHead = outargs[0];
    double pumpPower = outargs[1];
    double pumpEfficiency = outargs[2];
    
    std::cout << "  Tank HEAD: " << std::fixed << std::setprecision(2) << tankHead << " ft" << std::endl;
    std::cout << "  Pump POWER: " << pumpPower << " HP" << std::endl;
    std::cout << "  Pump EFFICIENCY: " << pumpEfficiency << " %" << std::endl;
    
    Assert(pumpPower >= 0.0, "Pump POWER is non-negative");
    Assert(pumpEfficiency >= 0.0 && pumpEfficiency <= 100.0, "Pump EFFICIENCY is in range [0, 100]");
    Assert(pumpEfficiency > 0.0, "Pump EFFICIENCY is positive when pump is ON");
    std::cout << std::endl;
    
    // Test 5: XF_CALCULATE with pump OFF (setting = 0.0)
    std::cout << "Test 5: XF_CALCULATE with pump OFF" << std::endl;
    inargs[0] = 600.0;  // ElapsedTime = 600 seconds (10 minutes)
    inargs[1] = 0.0;    // Pump setting = 0.0 (OFF)
    
    EpanetBridge(1, &status, inargs, outargs);
    Assert(status == 0, "XF_CALCULATE returns success");
    
    tankHead = outargs[0];
    pumpPower = outargs[1];
    pumpEfficiency = outargs[2];
    
    std::cout << "  Tank HEAD: " << std::fixed << std::setprecision(2) << tankHead << " ft" << std::endl;
    std::cout << "  Pump POWER: " << pumpPower << " HP" << std::endl;
    std::cout << "  Pump EFFICIENCY: " << pumpEfficiency << " %" << std::endl;
    
    Assert(pumpPower >= 0.0, "Pump POWER is non-negative");
    Assert(pumpEfficiency == 0.0, "Pump EFFICIENCY is 0 when pump is OFF");
    std::cout << std::endl;
    
    // Test 6: XF_CLEANUP
    std::cout << "Test 6: XF_CLEANUP" << std::endl;
    EpanetBridge(99, &status, inargs, outargs);
    Assert(status == 0, "XF_CLEANUP returns success");
    std::cout << std::endl;
    
    // Summary
    std::cout << "=== Test Summary ===" << std::endl;
    std::cout << "Tests passed: " << g_testsPassed << std::endl;
    std::cout << "Tests failed: " << g_testsFailed << std::endl;
    
    if (g_testsFailed == 0) {
        std::cout << std::endl;
        std::cout << "ALL TESTS PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << std::endl;
        std::cout << "SOME TESTS FAILED!" << std::endl;
        return 1;
    }
}
