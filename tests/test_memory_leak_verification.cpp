// Memory Leak Detection and Cleanup Verification Test
// Validates Requirements: 9.1, 9.2, 9.5, 9.7
//
// This test verifies proper memory management and resource cleanup by:
// 1. Running multiple initialize-cleanup cycles
// 2. Verifying file handles are closed after cleanup
// 3. Testing EPANET resource management
// 4. Validating exception safety

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include <psapi.h>

// Forward declarations for EpanetBridge
extern "C" void __declspec(dllexport) EpanetBridge(int methodID, int* status, double* inargs, double* outargs);

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

// Helper function to get current process memory usage
SIZE_T GetProcessMemoryUsage() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
}

// Helper function to check if a file is open
bool IsFileOpen(const std::string& filename) {
    // Try to open the file in exclusive mode
    HANDLE hFile = CreateFileA(
        filename.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, // No sharing - will fail if file is open
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        if (error == ERROR_SHARING_VIOLATION) {
            return true; // File is open by another process/handle
        }
        return false; // File doesn't exist or other error
    }
    
    CloseHandle(hFile);
    return false; // File exists but is not open
}

// Test 1: Multiple Initialize-Cleanup Cycles
bool TestMultipleInitCleanupCycles() {
    std::cout << "\n=== Test 1: Multiple Initialize-Cleanup Cycles ===" << std::endl;
    
    const int NUM_CYCLES = 10;
    std::vector<SIZE_T> memoryUsages;
    
    for (int cycle = 0; cycle < NUM_CYCLES; cycle++) {
        int status = XF_SUCCESS;
        double inargs[10] = {0};
        double outargs[10] = {0};
        
        // Initialize
        EpanetBridge(XF_INITIALIZE, &status, inargs, outargs);
        if (status != XF_SUCCESS) {
            std::cout << "FAIL: Initialization failed on cycle " << cycle << std::endl;
            return false;
        }
        
        // Run a few calculations
        for (int i = 0; i < 5; i++) {
            inargs[0] = i * 300.0; // ElapsedTime
            EpanetBridge(XF_CALCULATE, &status, inargs, outargs);
            if (status != XF_SUCCESS) {
                std::cout << "FAIL: Calculation failed on cycle " << cycle << ", step " << i << std::endl;
                return false;
            }
        }
        
        // Cleanup
        EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
        if (status != XF_SUCCESS) {
            std::cout << "FAIL: Cleanup failed on cycle " << cycle << std::endl;
            return false;
        }
        
        // Record memory usage after cleanup
        SIZE_T memUsage = GetProcessMemoryUsage();
        memoryUsages.push_back(memUsage);
        
        std::cout << "Cycle " << cycle << " completed. Memory: " 
                  << (memUsage / 1024) << " KB" << std::endl;
    }
    
    // Check for memory growth
    SIZE_T firstMemory = memoryUsages[0];
    SIZE_T lastMemory = memoryUsages[NUM_CYCLES - 1];
    SIZE_T memoryGrowth = lastMemory > firstMemory ? lastMemory - firstMemory : 0;
    
    // Allow up to 1 MB growth (some growth is expected due to heap fragmentation)
    const SIZE_T MAX_ALLOWED_GROWTH = 1024 * 1024; // 1 MB
    
    if (memoryGrowth > MAX_ALLOWED_GROWTH) {
        std::cout << "FAIL: Excessive memory growth detected: " 
                  << (memoryGrowth / 1024) << " KB" << std::endl;
        std::cout << "First cycle: " << (firstMemory / 1024) << " KB" << std::endl;
        std::cout << "Last cycle: " << (lastMemory / 1024) << " KB" << std::endl;
        return false;
    }
    
    std::cout << "PASS: Memory growth within acceptable limits: " 
              << (memoryGrowth / 1024) << " KB" << std::endl;
    return true;
}

// Test 2: File Handle Cleanup Verification
bool TestFileHandleCleanup() {
    std::cout << "\n=== Test 2: File Handle Cleanup Verification ===" << std::endl;
    
    const std::string logFile = "epanet_bridge_debug.log";
    
    int status = XF_SUCCESS;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Initialize (should open log file)
    EpanetBridge(XF_INITIALIZE, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        std::cout << "FAIL: Initialization failed" << std::endl;
        return false;
    }
    
    // Run a calculation
    inargs[0] = 300.0;
    EpanetBridge(XF_CALCULATE, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        std::cout << "FAIL: Calculation failed" << std::endl;
        return false;
    }
    
    // Cleanup (should close log file)
    EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        std::cout << "FAIL: Cleanup failed" << std::endl;
        return false;
    }
    
    // Give the system a moment to release the file handle
    Sleep(100);
    
    // Check if log file is still open
    if (IsFileOpen(logFile)) {
        std::cout << "FAIL: Log file is still open after cleanup" << std::endl;
        return false;
    }
    
    std::cout << "PASS: Log file properly closed after cleanup" << std::endl;
    return true;
}

// Test 3: EPANET Resource Cleanup
bool TestEpanetResourceCleanup() {
    std::cout << "\n=== Test 3: EPANET Resource Cleanup ===" << std::endl;
    
    int status = XF_SUCCESS;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Initialize
    EpanetBridge(XF_INITIALIZE, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        std::cout << "FAIL: Initialization failed" << std::endl;
        return false;
    }
    
    // Cleanup
    EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        std::cout << "FAIL: Cleanup failed" << std::endl;
        return false;
    }
    
    // Try to initialize again - should work if cleanup was proper
    EpanetBridge(XF_INITIALIZE, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        std::cout << "FAIL: Re-initialization failed after cleanup" << std::endl;
        return false;
    }
    
    // Cleanup again
    EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        std::cout << "FAIL: Second cleanup failed" << std::endl;
        return false;
    }
    
    std::cout << "PASS: EPANET resources properly cleaned up and re-initialized" << std::endl;
    return true;
}

// Test 4: Cleanup Without Initialization
bool TestCleanupWithoutInit() {
    std::cout << "\n=== Test 4: Cleanup Without Initialization ===" << std::endl;
    
    int status = XF_SUCCESS;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Call cleanup without initialization - should handle gracefully
    EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
    
    // Should succeed (or at least not crash)
    if (status != XF_SUCCESS) {
        std::cout << "WARNING: Cleanup without init returned error status" << std::endl;
        // This is acceptable - not a failure
    }
    
    std::cout << "PASS: Cleanup without initialization handled gracefully" << std::endl;
    return true;
}

// Test 5: Memory Stability Under Load
bool TestMemoryStabilityUnderLoad() {
    std::cout << "\n=== Test 5: Memory Stability Under Load ===" << std::endl;
    
    int status = XF_SUCCESS;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Initialize
    EpanetBridge(XF_INITIALIZE, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        std::cout << "FAIL: Initialization failed" << std::endl;
        return false;
    }
    
    SIZE_T initialMemory = GetProcessMemoryUsage();
    
    // Run many calculations
    const int NUM_CALCULATIONS = 100;
    for (int i = 0; i < NUM_CALCULATIONS; i++) {
        inargs[0] = i * 300.0; // ElapsedTime
        EpanetBridge(XF_CALCULATE, &status, inargs, outargs);
        if (status != XF_SUCCESS) {
            std::cout << "FAIL: Calculation failed at step " << i << std::endl;
            EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
            return false;
        }
    }
    
    SIZE_T finalMemory = GetProcessMemoryUsage();
    SIZE_T memoryGrowth = finalMemory > initialMemory ? finalMemory - initialMemory : 0;
    
    // Cleanup
    EpanetBridge(XF_CLEANUP, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        std::cout << "FAIL: Cleanup failed" << std::endl;
        return false;
    }
    
    // Allow up to 500 KB growth during calculations
    const SIZE_T MAX_ALLOWED_GROWTH = 512 * 1024; // 500 KB
    
    if (memoryGrowth > MAX_ALLOWED_GROWTH) {
        std::cout << "FAIL: Excessive memory growth during calculations: " 
                  << (memoryGrowth / 1024) << " KB" << std::endl;
        return false;
    }
    
    std::cout << "PASS: Memory stable during " << NUM_CALCULATIONS 
              << " calculations. Growth: " << (memoryGrowth / 1024) << " KB" << std::endl;
    return true;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Memory Leak Detection and Cleanup Verification" << std::endl;
    std::cout << "========================================" << std::endl;
    
    bool allPassed = true;
    
    // Run all tests
    allPassed &= TestMultipleInitCleanupCycles();
    allPassed &= TestFileHandleCleanup();
    allPassed &= TestEpanetResourceCleanup();
    allPassed &= TestCleanupWithoutInit();
    allPassed &= TestMemoryStabilityUnderLoad();
    
    std::cout << "\n========================================" << std::endl;
    if (allPassed) {
        std::cout << "ALL TESTS PASSED" << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } else {
        std::cout << "SOME TESTS FAILED" << std::endl;
        std::cout << "========================================" << std::endl;
        return 1;
    }
}
