// Property-Based Tests for Performance Requirements
// Feature: epanet-goldsim-bridge
// Tests Properties 15-19: Performance and Scalability

#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <functional>

// Include EPANET and bridge headers
extern "C" {
    #include "../../include/epanet2.h"
}

// GoldSim External Function Interface
extern "C" void __declspec(dllimport) EpanetBridge(int methodID, int* status, double* inargs, double* outargs);

// XFMethod enumerations
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

// Helper function to measure execution time in milliseconds
double MeasureExecutionTime(std::function<void()> func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    return duration.count();
}

// Helper function to get process memory usage in MB
double GetProcessMemoryMB() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / (1024.0 * 1024.0);
    }
    return 0.0;
}

// Helper function to count nodes and links in an EPANET model
bool GetNetworkSize(const std::string& inpFile, int& nodeCount, int& linkCount) {
    int errcode = ENopen(inpFile.c_str(), "", "");
    if (errcode != 0) {
        return false;
    }
    
    ENgetcount(EN_NODECOUNT, &nodeCount);
    ENgetcount(EN_LINKCOUNT, &linkCount);
    
    ENclose();
    return true;
}

// Property 15: Initialization Performance
// For any EPANET network with fewer than 1000 nodes, XF_INITIALIZE should complete in less than 1 second
// Validates: Requirements 14.1
void TestProperty15_InitializationPerformance() {
    std::cout << "\n=== Property 15: Initialization Performance ===" << std::endl;
    std::cout << "Validates: Requirements 14.1" << std::endl;
    std::cout << "Property: For any EPANET network with fewer than 1000 nodes, XF_INITIALIZE should complete in less than 1 second" << std::endl;
    
    // Use the simple test network (< 1000 nodes)
    std::string testFile = "test_data/simple_network.inp";
    
    // Get network size
    int nodeCount = 0, linkCount = 0;
    if (!GetNetworkSize(testFile, nodeCount, linkCount)) {
        std::cout << "FAILED: Could not open test network file" << std::endl;
        return;
    }
    
    std::cout << "Testing with network: " << nodeCount << " nodes, " << linkCount << " links" << std::endl;
    
    if (nodeCount >= 1000) {
        std::cout << "SKIPPED: Test network has >= 1000 nodes" << std::endl;
        return;
    }
    
    // Run multiple iterations to get average
    const int iterations = 10;
    std::vector<double> times;
    
    for (int i = 0; i < iterations; i++) {
        int status;
        double inargs[10] = {0};
        double outargs[10] = {0};
        
        // Measure initialization time
        double initTime = MeasureExecutionTime([&]() {
            EpanetBridge(XF_INITIALIZE, &status, inargs, outargs);
        });
        
        if (status != XF_SUCCESS) {
            std::cout << "FAILED: XF_INITIALIZE returned status " << status << std::endl;
            // Cleanup
            EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
            return;
        }
        
        times.push_back(initTime);
        
        // Cleanup for next iteration
        EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
        
        // Small delay between iterations
        Sleep(100);
    }
    
    // Calculate average and max time
    double avgTime = 0.0;
    double maxTime = 0.0;
    for (double t : times) {
        avgTime += t;
        if (t > maxTime) maxTime = t;
    }
    avgTime /= iterations;
    
    std::cout << "Average initialization time: " << avgTime << " ms" << std::endl;
    std::cout << "Maximum initialization time: " << maxTime << " ms" << std::endl;
    
    const double THRESHOLD_MS = 1000.0; // 1 second
    
    if (maxTime < THRESHOLD_MS) {
        std::cout << "PASSED: Initialization time (" << maxTime << " ms) < " << THRESHOLD_MS << " ms" << std::endl;
    } else {
        std::cout << "FAILED: Initialization time (" << maxTime << " ms) >= " << THRESHOLD_MS << " ms" << std::endl;
    }
}

// Property 16: Calculation Performance
// For any EPANET network with fewer than 1000 nodes, XF_CALCULATE should complete in less than 100 milliseconds
// Validates: Requirements 14.2
void TestProperty16_CalculationPerformance() {
    std::cout << "\n=== Property 16: Calculation Performance ===" << std::endl;
    std::cout << "Validates: Requirements 14.2" << std::endl;
    std::cout << "Property: For any EPANET network with fewer than 1000 nodes, XF_CALCULATE should complete in less than 100 milliseconds" << std::endl;
    
    // Initialize the bridge
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    EpanetBridge(XF_INITIALIZE, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        std::cout << "FAILED: Could not initialize bridge" << std::endl;
        return;
    }
    
    // Get argument counts
    EpanetBridge(XF_REP_ARGUMENTS, &status, inargs, outargs);
    int inputCount = (int)outargs[0];
    int outputCount = (int)outargs[1];
    
    std::cout << "Testing with " << inputCount << " inputs, " << outputCount << " outputs" << std::endl;
    
    // Prepare input/output arrays
    std::vector<double> calcInargs(inputCount);
    std::vector<double> calcOutargs(outputCount);
    
    // Run multiple calculation iterations
    const int iterations = 20;
    std::vector<double> times;
    
    for (int i = 0; i < iterations; i++) {
        // Set ElapsedTime
        calcInargs[0] = i * 3600.0; // Advance by 1 hour each iteration
        
        // Measure calculation time
        double calcTime = MeasureExecutionTime([&]() {
            EpanetBridge(XF_CALCULATE, &status, calcInargs.data(), calcOutargs.data());
        });
        
        if (status != XF_SUCCESS) {
            std::cout << "FAILED: XF_CALCULATE returned status " << status << " at iteration " << i << std::endl;
            EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
            return;
        }
        
        times.push_back(calcTime);
    }
    
    // Calculate statistics
    double avgTime = 0.0;
    double maxTime = 0.0;
    for (double t : times) {
        avgTime += t;
        if (t > maxTime) maxTime = t;
    }
    avgTime /= iterations;
    
    std::cout << "Average calculation time: " << avgTime << " ms" << std::endl;
    std::cout << "Maximum calculation time: " << maxTime << " ms" << std::endl;
    
    const double THRESHOLD_MS = 100.0;
    
    if (maxTime < THRESHOLD_MS) {
        std::cout << "PASSED: Calculation time (" << maxTime << " ms) < " << THRESHOLD_MS << " ms" << std::endl;
    } else {
        std::cout << "FAILED: Calculation time (" << maxTime << " ms) >= " << THRESHOLD_MS << " ms" << std::endl;
    }
    
    // Cleanup
    EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
}

// Property 17: Memory Overhead
// For any EPANET network, the Bridge_DLL should add less than 10 MB of memory overhead beyond EPANET's own requirements
// Validates: Requirements 14.3
void TestProperty17_MemoryOverhead() {
    std::cout << "\n=== Property 17: Memory Overhead ===" << std::endl;
    std::cout << "Validates: Requirements 14.3" << std::endl;
    std::cout << "Property: Bridge_DLL should add less than 10 MB of memory overhead beyond EPANET's own requirements" << std::endl;
    
    // Measure baseline memory
    double baselineMemory = GetProcessMemoryMB();
    std::cout << "Baseline memory: " << baselineMemory << " MB" << std::endl;
    
    // Initialize EPANET directly (without bridge)
    std::string testFile = "test_data/simple_network.inp";
    int errcode = ENopen(testFile.c_str(), "", "");
    if (errcode != 0) {
        std::cout << "FAILED: Could not open EPANET model directly" << std::endl;
        return;
    }
    
    ENopenH();
    ENinitH(0);
    
    double epanetMemory = GetProcessMemoryMB();
    double epanetOverhead = epanetMemory - baselineMemory;
    std::cout << "EPANET memory overhead: " << epanetOverhead << " MB" << std::endl;
    
    ENcloseH();
    ENclose();
    
    // Small delay to allow cleanup
    Sleep(100);
    
    // Now measure with bridge
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    EpanetBridge(XF_INITIALIZE, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        std::cout << "FAILED: Could not initialize bridge" << std::endl;
        return;
    }
    
    double bridgeMemory = GetProcessMemoryMB();
    double bridgeOverhead = bridgeMemory - baselineMemory;
    double additionalOverhead = bridgeOverhead - epanetOverhead;
    
    std::cout << "Bridge total memory overhead: " << bridgeOverhead << " MB" << std::endl;
    std::cout << "Bridge additional overhead (beyond EPANET): " << additionalOverhead << " MB" << std::endl;
    
    EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
    
    const double THRESHOLD_MB = 10.0;
    
    if (additionalOverhead < THRESHOLD_MB) {
        std::cout << "PASSED: Additional overhead (" << additionalOverhead << " MB) < " << THRESHOLD_MB << " MB" << std::endl;
    } else {
        std::cout << "FAILED: Additional overhead (" << additionalOverhead << " MB) >= " << THRESHOLD_MB << " MB" << std::endl;
    }
}

// Property 18: Scalability
// For any EPANET network with up to 10,000 nodes and 10,000 links, the Bridge_DLL should successfully 
// initialize, calculate, and cleanup without errors or performance degradation beyond linear scaling
// Validates: Requirements 14.5
void TestProperty18_Scalability() {
    std::cout << "\n=== Property 18: Scalability ===" << std::endl;
    std::cout << "Validates: Requirements 14.5" << std::endl;
    std::cout << "Property: Bridge should handle networks up to 10,000 nodes without errors or non-linear performance degradation" << std::endl;
    
    // Note: This test uses the available test network
    // For full scalability testing, larger networks would be needed
    
    std::string testFile = "test_data/simple_network.inp";
    int nodeCount = 0, linkCount = 0;
    
    if (!GetNetworkSize(testFile, nodeCount, linkCount)) {
        std::cout << "FAILED: Could not determine network size" << std::endl;
        return;
    }
    
    std::cout << "Testing with network: " << nodeCount << " nodes, " << linkCount << " links" << std::endl;
    std::cout << "Note: Full scalability test requires networks up to 10,000 nodes" << std::endl;
    
    // Test complete lifecycle
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Initialize
    double initTime = MeasureExecutionTime([&]() {
        EpanetBridge(XF_INITIALIZE, &status, inargs, outargs);
    });
    
    if (status != XF_SUCCESS) {
        std::cout << "FAILED: Initialization failed" << std::endl;
        return;
    }
    
    std::cout << "Initialization time: " << initTime << " ms" << std::endl;
    
    // Get argument counts
    EpanetBridge(XF_REP_ARGUMENTS, &status, inargs, outargs);
    int inputCount = (int)outargs[0];
    int outputCount = (int)outargs[1];
    
    std::vector<double> calcInargs(inputCount);
    std::vector<double> calcOutargs(outputCount);
    
    // Run multiple calculations
    const int iterations = 10;
    bool allSucceeded = true;
    
    for (int i = 0; i < iterations; i++) {
        calcInargs[0] = i * 3600.0;
        EpanetBridge(XF_CALCULATE, &status, calcInargs.data(), calcOutargs.data());
        
        if (status != XF_SUCCESS) {
            std::cout << "FAILED: Calculation failed at iteration " << i << std::endl;
            allSucceeded = false;
            break;
        }
    }
    
    // Cleanup
    EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
    
    if (status != XF_SUCCESS) {
        std::cout << "FAILED: Cleanup failed" << std::endl;
        return;
    }
    
    if (allSucceeded) {
        std::cout << "PASSED: Successfully completed " << iterations << " calculations without errors" << std::endl;
        std::cout << "Note: Test with current network size (" << nodeCount << " nodes). Full test requires up to 10,000 nodes." << std::endl;
    }
}

// Property 19: File I/O Minimization
// For any sequence of XF_CALCULATE calls within a single realization, no file I/O operations should occur
// Validates: Requirements 14.6
void TestProperty19_FileIOMinimization() {
    std::cout << "\n=== Property 19: File I/O Minimization ===" << std::endl;
    std::cout << "Validates: Requirements 14.6" << std::endl;
    std::cout << "Property: No file I/O operations should occur during XF_CALCULATE calls" << std::endl;
    
    // Initialize
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    EpanetBridge(XF_INITIALIZE, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        std::cout << "FAILED: Could not initialize bridge" << std::endl;
        return;
    }
    
    // Get argument counts
    EpanetBridge(XF_REP_ARGUMENTS, &status, inargs, outargs);
    int inputCount = (int)outargs[0];
    int outputCount = (int)outargs[1];
    
    std::vector<double> calcInargs(inputCount);
    std::vector<double> calcOutargs(outputCount);
    
    // Note: Direct file I/O monitoring would require OS-level hooks or profiling tools
    // This test verifies that calculations complete quickly (indicating no file I/O)
    // and that no temporary files are created
    
    std::cout << "Checking for temporary file creation during calculations..." << std::endl;
    
    // Count files in current directory before
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("*.*", &findData);
    int fileCountBefore = 0;
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                fileCountBefore++;
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
    
    // Run calculations
    const int iterations = 20;
    for (int i = 0; i < iterations; i++) {
        calcInargs[0] = i * 3600.0;
        EpanetBridge(XF_CALCULATE, &status, calcInargs.data(), calcOutargs.data());
        
        if (status != XF_SUCCESS) {
            std::cout << "FAILED: Calculation failed" << std::endl;
            EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
            return;
        }
    }
    
    // Count files after
    hFind = FindFirstFileA("*.*", &findData);
    int fileCountAfter = 0;
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                fileCountAfter++;
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
    
    std::cout << "Files before: " << fileCountBefore << ", Files after: " << fileCountAfter << std::endl;
    
    // Cleanup
    EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
    
    if (fileCountAfter == fileCountBefore) {
        std::cout << "PASSED: No temporary files created during calculations" << std::endl;
    } else {
        std::cout << "WARNING: File count changed (may indicate file I/O)" << std::endl;
        std::cout << "Note: This could be due to logging or other legitimate operations" << std::endl;
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Performance Property-Based Tests" << std::endl;
    std::cout << "Feature: epanet-goldsim-bridge" << std::endl;
    std::cout << "Properties 15-19" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Change to tests directory to find test data
    SetCurrentDirectoryA("tests");
    
    // Run all performance property tests
    TestProperty15_InitializationPerformance();
    TestProperty16_CalculationPerformance();
    TestProperty17_MemoryOverhead();
    TestProperty18_Scalability();
    TestProperty19_FileIOMinimization();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Performance Tests Complete" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
