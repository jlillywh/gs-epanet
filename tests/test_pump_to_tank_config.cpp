/**
 * @file test_pump_to_tank_config.cpp
 * @brief Quick test to verify Pump_to_Tank configuration with POWER and EFFICIENCY
 */

#include <iostream>
#include <iomanip>
#include "../src/EpanetBridge.h"

int main() {
    std::cout << "=== Test Pump_to_Tank Configuration ===" << std::endl;
    std::cout << std::endl;
    
    int status = 0;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Test XF_REP_VERSION
    std::cout << "XF_REP_VERSION:" << std::endl;
    EpanetBridge(2, &status, inargs, outargs);
    std::cout << "  Status: " << status << std::endl;
    std::cout << "  Version: " << outargs[0] << std::endl;
    std::cout << std::endl;
    
    // Test XF_REP_ARGUMENTS
    std::cout << "XF_REP_ARGUMENTS:" << std::endl;
    EpanetBridge(3, &status, inargs, outargs);
    std::cout << "  Status: " << status << std::endl;
    std::cout << "  Input count: " << (int)outargs[0] << std::endl;
    std::cout << "  Output count: " << (int)outargs[1] << std::endl;
    std::cout << std::endl;
    
    if ((int)outargs[0] != 3 || (int)outargs[1] != 7) {
        std::cout << "ERROR: Expected 3 inputs and 7 outputs!" << std::endl;
        return 1;
    }
    
    // Test XF_INITIALIZE
    std::cout << "XF_INITIALIZE:" << std::endl;
    EpanetBridge(0, &status, inargs, outargs);
    std::cout << "  Status: " << status << std::endl;
    if (status != 0) {
        std::cout << "ERROR: Initialization failed!" << std::endl;
        return 1;
    }
    std::cout << std::endl;
    
    // Test XF_CALCULATE with pump ON
    std::cout << "XF_CALCULATE (pump ON):" << std::endl;
    inargs[0] = 300.0;  // ElapsedTime = 5 minutes
    inargs[1] = 1.0;    // Pump setting = 1.0 (ON)
    inargs[2] = 1.0;    // Emitter coefficient
    
    EpanetBridge(1, &status, inargs, outargs);
    std::cout << "  Status: " << status << std::endl;
    if (status != 0) {
        std::cout << "ERROR: Calculate failed!" << std::endl;
        return 1;
    }
    
    std::cout << "  Output[0] Tank1 HEAD: " << std::fixed << std::setprecision(2) << outargs[0] << " ft" << std::endl;
    std::cout << "  Output[1] J2 PRESSURE: " << outargs[1] << " psi" << std::endl;
    std::cout << "  Output[2] Pump1 FLOW: " << outargs[2] << " GPM" << std::endl;
    std::cout << "  Output[3] Pipe 1 FLOW: " << outargs[3] << " GPM" << std::endl;
    std::cout << "  Output[4] Pipe 2 FLOW: " << outargs[4] << " GPM" << std::endl;
    std::cout << "  Output[5] Pump1 POWER: " << outargs[5] << " HP" << std::endl;
    std::cout << "  Output[6] Pump1 EFFICIENCY: " << outargs[6] << " %" << std::endl;
    std::cout << std::endl;
    
    // Test XF_CLEANUP
    std::cout << "XF_CLEANUP:" << std::endl;
    EpanetBridge(99, &status, inargs, outargs);
    std::cout << "  Status: " << status << std::endl;
    std::cout << std::endl;
    
    std::cout << "=== ALL TESTS PASSED ===" << std::endl;
    return 0;
}
