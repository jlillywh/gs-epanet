/**
 * @file test_tank_level_validation.cpp
 * @brief Validates that tank levels update correctly over multiple timesteps
 * 
 * This test uses the Pump_to_Tank example to verify that:
 * 1. Tank levels change over time when there is net inflow/outflow
 * 2. Tank levels increase when inflow > outflow
 * 3. Tank levels are not stuck at initial values
 * 
 * Test Setup:
 * - Pump at 100% speed (~100 GPM inflow)
 * - Small emitter coefficient (~1 GPM outflow)
 * - Net inflow ~99 GPM should cause tank to rise
 * 
 * Expected Results:
 * - Tank level should increase by ~0.03-0.05 ft per 5-minute timestep
 * - Tank level at t=10min should be > tank level at t=5min
 * - Tank level at t=15min should be > tank level at t=10min
 */

#include <windows.h>
#include <iostream>
#include <iomanip>
#include <cmath>

// GoldSim External Function signature
typedef void (*EpanetBridgeFunc)(int methodID, int* status, double* inargs, double* outargs);

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
    XF_CLEANUP_NOW = 99
};

int main() {
    std::cout << "========================================\n";
    std::cout << "Tank Level Update Validation Test\n";
    std::cout << "========================================\n\n";

    // Load the DLL
    HMODULE hDll = LoadLibraryA("gs_epanet.dll");
    if (!hDll) {
        std::cerr << "ERROR: Failed to load gs_epanet.dll\n";
        std::cerr << "Make sure the DLL is in the current directory.\n";
        return 1;
    }

    // Get the function pointer
    EpanetBridgeFunc EpanetBridge = (EpanetBridgeFunc)GetProcAddress(hDll, "EpanetBridge");
    if (!EpanetBridge) {
        std::cerr << "ERROR: Failed to find EpanetBridge function in DLL\n";
        FreeLibrary(hDll);
        return 1;
    }

    std::cout << "DLL loaded successfully.\n\n";

    // Query version
    int status = 0;
    double inargs[10] = {0};
    double outargs[10] = {0};

    std::cout << "STEP 1: Query DLL Version\n";
    EpanetBridge(XF_REP_VERSION, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        std::cerr << "ERROR: XF_REP_VERSION failed\n";
        FreeLibrary(hDll);
        return 1;
    }
    std::cout << "  DLL Version: " << std::fixed << std::setprecision(2) << outargs[0] << "\n\n";

    // Query argument counts
    std::cout << "STEP 2: Query Input/Output Counts\n";
    EpanetBridge(XF_REP_ARGUMENTS, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        std::cerr << "ERROR: XF_REP_ARGUMENTS failed\n";
        FreeLibrary(hDll);
        return 1;
    }
    int numInputs = (int)outargs[0];
    int numOutputs = (int)outargs[1];
    std::cout << "  Inputs: " << numInputs << "\n";
    std::cout << "  Outputs: " << numOutputs << "\n\n";

    // Initialize
    std::cout << "STEP 3: Initialize EPANET Model\n";
    EpanetBridge(XF_INITIALIZE, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        std::cerr << "ERROR: XF_INITIALIZE failed\n";
        FreeLibrary(hDll);
        return 1;
    }
    std::cout << "  Initialization successful.\n\n";

    // Run 3 timesteps and track tank level
    std::cout << "STEP 4: Run 3 Timesteps and Monitor Tank Level\n";
    std::cout << "========================================\n\n";

    const int NUM_TIMESTEPS = 3;
    const double TIMESTEP_SECONDS = 300.0;  // 5 minutes
    double tankLevels[NUM_TIMESTEPS];
    bool testPassed = true;

    for (int t = 0; t < NUM_TIMESTEPS; t++) {
        // Set inputs
        inargs[0] = (t + 1) * TIMESTEP_SECONDS;  // ElapsedTime
        inargs[1] = 1.0;                          // Pump1 setting (100% speed)
        inargs[2] = 0.5;                          // Outlet emitter coefficient

        // Call XF_CALCULATE
        EpanetBridge(XF_CALCULATE, &status, inargs, outargs);
        if (status != XF_SUCCESS) {
            std::cerr << "ERROR: XF_CALCULATE failed at timestep " << (t + 1) << "\n";
            testPassed = false;
            break;
        }

        // Extract outputs
        double tankLevel = outargs[0];      // Tank1 level
        double pressure = outargs[1];       // J2 pressure
        double pumpFlow = outargs[2];       // Pump1 flow
        double outletFlow = outargs[3];     // Pipe 1 flow (out of tank)
        double inletFlow = outargs[4];      // Pipe 2 flow (into tank)

        tankLevels[t] = tankLevel;

        // Display results
        std::cout << "Timestep " << (t + 1) << " (t = " << (inargs[0] / 60.0) << " min):\n";
        std::cout << "  Tank Level:    " << std::fixed << std::setprecision(4) << tankLevel << " ft\n";
        std::cout << "  Pump Flow:     " << std::fixed << std::setprecision(2) << pumpFlow << " GPM\n";
        std::cout << "  Inlet Flow:    " << std::fixed << std::setprecision(2) << inletFlow << " GPM (into tank)\n";
        std::cout << "  Outlet Flow:   " << std::fixed << std::setprecision(2) << outletFlow << " GPM (out of tank)\n";
        std::cout << "  Net Inflow:    " << std::fixed << std::setprecision(2) << (inletFlow - outletFlow) << " GPM\n";
        std::cout << "  J2 Pressure:   " << std::fixed << std::setprecision(2) << pressure << " psi\n";
        std::cout << "\n";
    }

    // Cleanup
    std::cout << "STEP 5: Cleanup\n";
    EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
    if (status != XF_SUCCESS && status != XF_CLEANUP_NOW) {
        std::cerr << "WARNING: XF_CLEANUP returned status " << status << "\n";
    }
    std::cout << "  Cleanup successful.\n\n";

    // Validate results
    std::cout << "========================================\n";
    std::cout << "VALIDATION RESULTS\n";
    std::cout << "========================================\n\n";

    if (!testPassed) {
        std::cout << "TEST FAILED: Simulation error occurred\n";
        FreeLibrary(hDll);
        return 1;
    }

    // Check 1: Tank level should not be constant
    bool allSame = true;
    for (int t = 1; t < NUM_TIMESTEPS; t++) {
        if (std::abs(tankLevels[t] - tankLevels[0]) > 0.001) {
            allSame = false;
            break;
        }
    }

    if (allSame) {
        std::cout << "FAIL: Tank level is constant at " << tankLevels[0] << " ft\n";
        std::cout << "      Tank levels are not being updated!\n";
        testPassed = false;
    } else {
        std::cout << "PASS: Tank level changes over time\n";
    }

    // Check 2: Tank level should increase (net inflow is positive)
    bool increasing = true;
    for (int t = 1; t < NUM_TIMESTEPS; t++) {
        if (tankLevels[t] <= tankLevels[t-1]) {
            increasing = false;
            break;
        }
    }

    if (!increasing) {
        std::cout << "FAIL: Tank level is not increasing\n";
        std::cout << "      Expected increasing levels with net inflow\n";
        testPassed = false;
    } else {
        std::cout << "PASS: Tank level increases over time\n";
    }

    // Check 3: Rate of change should be reasonable
    double totalChange = tankLevels[NUM_TIMESTEPS-1] - tankLevels[0];
    double expectedChange = 0.05;  // Rough estimate: ~0.03-0.05 ft per timestep
    
    if (totalChange < 0.01) {
        std::cout << "FAIL: Tank level change too small (" << totalChange << " ft)\n";
        std::cout << "      Expected at least 0.01 ft change over " << NUM_TIMESTEPS << " timesteps\n";
        testPassed = false;
    } else {
        std::cout << "PASS: Tank level change is reasonable (" << totalChange << " ft)\n";
    }

    std::cout << "\n";
    std::cout << "Tank Level Summary:\n";
    for (int t = 0; t < NUM_TIMESTEPS; t++) {
        std::cout << "  t=" << ((t+1) * TIMESTEP_SECONDS / 60.0) << " min: " 
                  << std::fixed << std::setprecision(4) << tankLevels[t] << " ft";
        if (t > 0) {
            double change = tankLevels[t] - tankLevels[t-1];
            std::cout << " (change: " << std::showpos << change << std::noshowpos << " ft)";
        }
        std::cout << "\n";
    }

    std::cout << "\n========================================\n";
    if (testPassed) {
        std::cout << "ALL TESTS PASSED\n";
    } else {
        std::cout << "SOME TESTS FAILED\n";
    }
    std::cout << "========================================\n";

    FreeLibrary(hDll);
    return testPassed ? 0 : 1;
}
