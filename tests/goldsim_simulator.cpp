/**
 * @file goldsim_simulator.cpp
 * @brief Simulates GoldSim calling the EPANET Bridge DLL
 * 
 * This program acts as a 32-bit GoldSim application, demonstrating the
 * complete lifecycle of calling the EpanetBridge DLL through multiple timesteps.
 * 
 * Compilation (32-bit):
 *   cl /EHsc /Fe:goldsim_simulator.exe goldsim_simulator.cpp
 * 
 * Usage:
 *   goldsim_simulator.exe [path_to_gs_epanet.dll] [num_timesteps]
 */

#include <windows.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

// GoldSim External Function API constants
enum XFMethodID {
    XF_INITIALIZE = 0,
    XF_CALCULATE = 1,
    XF_REP_VERSION = 2,
    XF_REP_ARGUMENTS = 3,
    XF_CLEANUP = 99
};

enum XFStatusID {
    XF_SUCCESS = 0,
    XF_FAILURE = 1,
    XF_CLEANUP_NOW = 99,
    XF_FAILURE_WITH_MSG = -1,
    XF_INCREASE_MEMORY = -2
};

// Function pointer type for the DLL entry point
typedef void (*EpanetBridgeFunc)(int methodID, int* status, double* inargs, double* outargs);

/**
 * @brief Simulates time-varying demand for a junction
 * @param timestep Current timestep (0-based)
 * @param baseValue Base demand value
 * @param pattern Pattern type (0=residential, 1=commercial, 2=industrial)
 * @return Modified demand value
 */
double SimulateDemand(int timestep, double baseValue, int pattern) {
    // Simulate hourly patterns (assuming 1-hour timesteps)
    double multiplier = 1.0;
    int hour = timestep % 24;
    
    switch (pattern) {
        case 0: // Residential (morning/evening peaks)
            if (hour >= 6 && hour < 9) multiplier = 1.3;
            else if (hour >= 17 && hour < 20) multiplier = 1.5;
            else if (hour >= 0 && hour < 6) multiplier = 0.5;
            else multiplier = 1.0;
            break;
            
        case 1: // Commercial (daytime peak)
            if (hour >= 9 && hour < 17) multiplier = 1.4;
            else if (hour >= 6 && hour < 9) multiplier = 0.8;
            else multiplier = 0.3;
            break;
            
        case 2: // Industrial (constant with shift changes)
            if (hour >= 7 && hour < 19) multiplier = 1.2;
            else multiplier = 0.8;
            break;
    }
    
    return baseValue * multiplier;
}

/**
 * @brief Main simulator function
 */
int main(int argc, char* argv[]) {
    std::cout << "========================================\n";
    std::cout << "GoldSim EPANET Bridge Simulator\n";
    std::cout << "========================================\n\n";
    
    // Parse command line arguments
    const char* dllPath = (argc > 1) ? argv[1] : "gs_epanet.dll";
    int numTimesteps = (argc > 2) ? atoi(argv[2]) : 10;
    
    std::cout << "DLL Path: " << dllPath << "\n";
    std::cout << "Number of Timesteps: " << numTimesteps << "\n\n";
    
    // Load the DLL
    std::cout << "Loading DLL...\n";
    HMODULE hDll = LoadLibraryA(dllPath);
    if (!hDll) {
        std::cerr << "ERROR: Failed to load DLL. Error code: " << GetLastError() << "\n";
        std::cerr << "Make sure gs_epanet.dll is in the same directory or in PATH.\n";
        return 1;
    }
    std::cout << "DLL loaded successfully.\n\n";
    
    // Get the function pointer
    EpanetBridgeFunc EpanetBridge = (EpanetBridgeFunc)GetProcAddress(hDll, "EpanetBridge");
    if (!EpanetBridge) {
        std::cerr << "ERROR: Failed to find EpanetBridge function in DLL.\n";
        FreeLibrary(hDll);
        return 1;
    }
    std::cout << "Function 'EpanetBridge' found.\n\n";
    
    // ========================================
    // STEP 1: Report Version
    // ========================================
    std::cout << "STEP 1: Querying DLL Version (XF_REP_VERSION)\n";
    std::cout << "----------------------------------------\n";
    
    int status = XF_SUCCESS;
    double outargs[100] = {0};
    
    EpanetBridge(XF_REP_VERSION, &status, nullptr, outargs);
    
    if (status != XF_SUCCESS) {
        std::cerr << "ERROR: XF_REP_VERSION failed with status " << status << "\n";
        FreeLibrary(hDll);
        return 1;
    }
    
    std::cout << "DLL Version: " << std::fixed << std::setprecision(2) << outargs[0] << "\n\n";
    
    // ========================================
    // STEP 2: Report Arguments
    // ========================================
    std::cout << "STEP 2: Querying Input/Output Counts (XF_REP_ARGUMENTS)\n";
    std::cout << "----------------------------------------\n";
    
    memset(outargs, 0, sizeof(outargs));
    EpanetBridge(XF_REP_ARGUMENTS, &status, nullptr, outargs);
    
    if (status != XF_SUCCESS) {
        std::cerr << "ERROR: XF_REP_ARGUMENTS failed with status " << status << "\n";
        FreeLibrary(hDll);
        return 1;
    }
    
    int numInputs = (int)outargs[0];
    int numOutputs = (int)outargs[1];
    
    std::cout << "Number of Inputs: " << numInputs << "\n";
    std::cout << "Number of Outputs: " << numOutputs << "\n\n";
    
    // Allocate arrays based on reported counts
    std::vector<double> inargs(numInputs, 0.0);
    std::vector<double> outputs(numOutputs, 0.0);
    
    // ========================================
    // STEP 3: Initialize
    // ========================================
    std::cout << "STEP 3: Initializing EPANET Model (XF_INITIALIZE)\n";
    std::cout << "----------------------------------------\n";
    
    // Set initial elapsed time to 0
    inargs[0] = 0.0;
    
    // Set initial demands (Example2 base values)
    if (numInputs >= 4) {
        inargs[1] = 50.0;  // J1 demand (GPM)
        inargs[2] = 45.0;  // J5 demand (GPM)
        inargs[3] = 85.0;  // J10 demand (GPM)
    }
    
    EpanetBridge(XF_INITIALIZE, &status, inargs.data(), outputs.data());
    
    if (status == XF_FAILURE_WITH_MSG) {
        char* errorMsg = (char*)(*(ULONG_PTR*)outputs.data());
        std::cerr << "ERROR: Initialization failed: " << errorMsg << "\n";
        FreeLibrary(hDll);
        return 1;
    } else if (status != XF_SUCCESS) {
        std::cerr << "ERROR: Initialization failed with status " << status << "\n";
        FreeLibrary(hDll);
        return 1;
    }
    
    std::cout << "Initialization successful.\n\n";
    
    // ========================================
    // STEP 4: Run Timesteps
    // ========================================
    std::cout << "STEP 4: Running " << numTimesteps << " Timesteps (XF_CALCULATE)\n";
    std::cout << "========================================\n\n";
    
    // Timestep is 3600 seconds (1 hour) based on Example2 config
    const double timestepSeconds = 3600.0;
    
    for (int t = 0; t < numTimesteps; t++) {
        std::cout << "Timestep " << (t + 1) << " / " << numTimesteps << "\n";
        std::cout << "----------------------------------------\n";
        
        // Update elapsed time
        inargs[0] = t * timestepSeconds;
        
        // Simulate time-varying demands
        if (numInputs >= 4) {
            inargs[1] = SimulateDemand(t, 50.0, 0);  // J1 (residential)
            inargs[2] = SimulateDemand(t, 45.0, 1);  // J5 (commercial)
            inargs[3] = SimulateDemand(t, 85.0, 2);  // J10 (industrial)
        }
        
        std::cout << "Inputs:\n";
        std::cout << "  ElapsedTime: " << std::fixed << std::setprecision(0) 
                  << inargs[0] << " sec (" << (inargs[0] / 3600.0) << " hrs)\n";
        if (numInputs >= 4) {
            std::cout << "  J1 Demand:   " << std::fixed << std::setprecision(2) 
                      << inargs[1] << " GPM\n";
            std::cout << "  J5 Demand:   " << std::fixed << std::setprecision(2) 
                      << inargs[2] << " GPM\n";
            std::cout << "  J10 Demand:  " << std::fixed << std::setprecision(2) 
                      << inargs[3] << " GPM\n";
        }
        
        // Call XF_CALCULATE
        memset(outputs.data(), 0, outputs.size() * sizeof(double));
        EpanetBridge(XF_CALCULATE, &status, inargs.data(), outputs.data());
        
        if (status == XF_FAILURE_WITH_MSG) {
            char* errorMsg = (char*)(*(ULONG_PTR*)outputs.data());
            std::cerr << "ERROR at timestep " << (t + 1) << ": " << errorMsg << "\n";
            break;
        } else if (status != XF_SUCCESS) {
            std::cerr << "ERROR: Calculation failed at timestep " << (t + 1) 
                      << " with status " << status << "\n";
            break;
        }
        
        // Display outputs
        std::cout << "\nOutputs:\n";
        if (numOutputs >= 10) {
            std::cout << "  Junction Pressures (psi):\n";
            for (int i = 0; i < 10; i++) {
                std::cout << "    J" << (i + 1) << ": " << std::fixed 
                          << std::setprecision(2) << outputs[i] << " psi\n";
            }
        }
        if (numOutputs >= 12) {
            std::cout << "  Pipe Flows (GPM):\n";
            std::cout << "    P1: " << std::fixed << std::setprecision(2) 
                      << outputs[10] << " GPM\n";
            std::cout << "    P6: " << std::fixed << std::setprecision(2) 
                      << outputs[11] << " GPM\n";
        }
        
        std::cout << "\n";
    }
    
    // ========================================
    // STEP 5: Cleanup
    // ========================================
    std::cout << "STEP 5: Cleaning Up (XF_CLEANUP)\n";
    std::cout << "----------------------------------------\n";
    
    EpanetBridge(XF_CLEANUP, &status, nullptr, nullptr);
    
    if (status != XF_SUCCESS && status != XF_CLEANUP_NOW) {
        std::cerr << "WARNING: Cleanup returned status " << status << "\n";
    } else {
        std::cout << "Cleanup successful.\n";
    }
    
    // Unload DLL
    std::cout << "\nUnloading DLL...\n";
    FreeLibrary(hDll);
    
    std::cout << "\n========================================\n";
    std::cout << "Simulation Complete!\n";
    std::cout << "========================================\n";
    
    return 0;
}
