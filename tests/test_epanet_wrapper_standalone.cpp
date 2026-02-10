/**
 * @file test_epanet_wrapper_standalone.cpp
 * @brief Standalone test for EpanetWrapper hydraulic solver methods
 * 
 * Tests task 4.2: Implement hydraulic solver methods
 * - SolveHydraulics() to call ENrunH()
 * - NextHydraulicStep() to call ENnextH()
 * - Error checking and logging for all EPANET calls
 */

#include <iostream>
#include <string>
#include <cassert>
#include "../src/EpanetWrapper.h"

// Test result tracking
int g_testsPassed = 0;
int g_testsFailed = 0;

// Helper macros
#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "FAIL: " << message << std::endl; \
        g_testsFailed++; \
        return false; \
    } else { \
        g_testsPassed++; \
    }

#define RUN_TEST(test_func) \
    std::cout << "Running " << #test_func << "..." << std::endl; \
    if (test_func()) { \
        std::cout << "PASS: " << #test_func << std::endl; \
    } else { \
        std::cout << "FAIL: " << #test_func << std::endl; \
    }

/**
 * Test 1: SolveHydraulics returns error when hydraulics not initialized
 */
bool Test_SolveHydraulics_NotInitialized() {
    long currentTime = 0;
    int errorCode = EpanetWrapper::SolveHydraulics(&currentTime);
    
    TEST_ASSERT(errorCode == 103, 
                "SolveHydraulics should return 103 (hydraulics not initialized) when called before initialization");
    
    std::string errorMsg = EpanetWrapper::GetErrorMessage(errorCode);
    TEST_ASSERT(errorMsg.find("not initialized") != std::string::npos,
                "Error message should mention 'not initialized'");
    
    return true;
}

/**
 * Test 2: NextHydraulicStep returns error when hydraulics not initialized
 */
bool Test_NextHydraulicStep_NotInitialized() {
    long nextTime = 0;
    int errorCode = EpanetWrapper::NextHydraulicStep(&nextTime);
    
    TEST_ASSERT(errorCode == 103,
                "NextHydraulicStep should return 103 (hydraulics not initialized) when called before initialization");
    
    std::string errorMsg = EpanetWrapper::GetErrorMessage(errorCode);
    TEST_ASSERT(errorMsg.find("not initialized") != std::string::npos,
                "Error message should mention 'not initialized'");
    
    return true;
}

/**
 * Test 3: Complete hydraulic lifecycle with SolveHydraulics and NextHydraulicStep
 */
bool Test_HydraulicLifecycle_Complete() {
    // Open EPANET model
    int errorCode = EpanetWrapper::Open("test_data/simple_test.inp", "", "");
    TEST_ASSERT(errorCode == 0, 
                "Open should succeed with valid .inp file. Error: " + EpanetWrapper::GetErrorMessage(errorCode));
    TEST_ASSERT(EpanetWrapper::IsOpen(), "IsOpen should return true after successful Open");
    
    // Open hydraulics
    errorCode = EpanetWrapper::OpenHydraulics();
    TEST_ASSERT(errorCode == 0,
                "OpenHydraulics should succeed. Error: " + EpanetWrapper::GetErrorMessage(errorCode));
    TEST_ASSERT(EpanetWrapper::IsHydraulicsOpen(), "IsHydraulicsOpen should return true after successful OpenHydraulics");
    
    // Initialize hydraulics
    errorCode = EpanetWrapper::InitHydraulics();
    TEST_ASSERT(errorCode == 0,
                "InitHydraulics should succeed. Error: " + EpanetWrapper::GetErrorMessage(errorCode));
    
    // Solve hydraulics for first timestep
    long currentTime = 0;
    errorCode = EpanetWrapper::SolveHydraulics(&currentTime);
    TEST_ASSERT(errorCode == 0,
                "SolveHydraulics should succeed for first timestep. Error: " + EpanetWrapper::GetErrorMessage(errorCode));
    TEST_ASSERT(currentTime == 0, "First timestep should be at time 0");
    
    std::cout << "  First timestep solved at time: " << currentTime << " seconds" << std::endl;
    
    // Advance to next timestep
    long nextTime = 0;
    errorCode = EpanetWrapper::NextHydraulicStep(&nextTime);
    TEST_ASSERT(errorCode == 0,
                "NextHydraulicStep should succeed. Error: " + EpanetWrapper::GetErrorMessage(errorCode));
    TEST_ASSERT(nextTime > 0, "Next timestep should be greater than 0");
    
    std::cout << "  Next hydraulic timestep: " << nextTime << " seconds" << std::endl;
    
    // Solve hydraulics for second timestep
    errorCode = EpanetWrapper::SolveHydraulics(&currentTime);
    TEST_ASSERT(errorCode == 0,
                "SolveHydraulics should succeed for second timestep. Error: " + EpanetWrapper::GetErrorMessage(errorCode));
    TEST_ASSERT(currentTime == nextTime, "Current time should match next time from previous step");
    
    std::cout << "  Second timestep solved at time: " << currentTime << " seconds" << std::endl;
    
    // Advance again
    errorCode = EpanetWrapper::NextHydraulicStep(&nextTime);
    TEST_ASSERT(errorCode == 0,
                "NextHydraulicStep should succeed for second advance. Error: " + EpanetWrapper::GetErrorMessage(errorCode));
    
    std::cout << "  Third hydraulic timestep: " << nextTime << " seconds" << std::endl;
    
    // Close hydraulics
    errorCode = EpanetWrapper::CloseHydraulics();
    TEST_ASSERT(errorCode == 0,
                "CloseHydraulics should succeed. Error: " + EpanetWrapper::GetErrorMessage(errorCode));
    TEST_ASSERT(!EpanetWrapper::IsHydraulicsOpen(), "IsHydraulicsOpen should return false after CloseHydraulics");
    
    // Close EPANET
    errorCode = EpanetWrapper::Close();
    TEST_ASSERT(errorCode == 0,
                "Close should succeed. Error: " + EpanetWrapper::GetErrorMessage(errorCode));
    TEST_ASSERT(!EpanetWrapper::IsOpen(), "IsOpen should return false after Close");
    
    return true;
}

/**
 * Test 4: Multiple timesteps in a loop
 */
bool Test_MultipleTimesteps_Loop() {
    // Open and initialize
    int errorCode = EpanetWrapper::Open("test_data/simple_test.inp", "", "");
    TEST_ASSERT(errorCode == 0, "Open should succeed");
    
    errorCode = EpanetWrapper::OpenHydraulics();
    TEST_ASSERT(errorCode == 0, "OpenHydraulics should succeed");
    
    errorCode = EpanetWrapper::InitHydraulics();
    TEST_ASSERT(errorCode == 0, "InitHydraulics should succeed");
    
    // Run simulation for 5 timesteps
    const int numTimesteps = 5;
    long currentTime = 0;
    long nextTime = 0;
    
    for (int i = 0; i < numTimesteps; i++) {
        // Solve hydraulics
        errorCode = EpanetWrapper::SolveHydraulics(&currentTime);
        TEST_ASSERT(errorCode == 0,
                    "SolveHydraulics should succeed at timestep " + std::to_string(i));
        
        std::cout << "  Timestep " << i << ": time = " << currentTime << " seconds" << std::endl;
        
        // Advance to next timestep
        errorCode = EpanetWrapper::NextHydraulicStep(&nextTime);
        TEST_ASSERT(errorCode == 0,
                    "NextHydraulicStep should succeed at timestep " + std::to_string(i));
        
        // Check if we've reached the end of simulation
        if (nextTime == 0) {
            std::cout << "  Reached end of simulation at timestep " << i << std::endl;
            break;
        }
    }
    
    // Cleanup
    errorCode = EpanetWrapper::CloseHydraulics();
    TEST_ASSERT(errorCode == 0, "CloseHydraulics should succeed");
    
    errorCode = EpanetWrapper::Close();
    TEST_ASSERT(errorCode == 0, "Close should succeed");
    
    return true;
}

/**
 * Test 5: Get node values after solving hydraulics
 */
bool Test_GetNodeValues_AfterSolve() {
    // Open and initialize
    int errorCode = EpanetWrapper::Open("test_data/simple_test.inp", "", "");
    TEST_ASSERT(errorCode == 0, "Open should succeed");
    
    errorCode = EpanetWrapper::OpenHydraulics();
    TEST_ASSERT(errorCode == 0, "OpenHydraulics should succeed");
    
    errorCode = EpanetWrapper::InitHydraulics();
    TEST_ASSERT(errorCode == 0, "InitHydraulics should succeed");
    
    // Solve hydraulics
    long currentTime = 0;
    errorCode = EpanetWrapper::SolveHydraulics(&currentTime);
    TEST_ASSERT(errorCode == 0, "SolveHydraulics should succeed");
    
    // Get node index for J1
    int nodeIndex = 0;
    errorCode = EpanetWrapper::GetNodeIndex("J1", &nodeIndex);
    TEST_ASSERT(errorCode == 0, "GetNodeIndex should succeed for J1");
    TEST_ASSERT(nodeIndex > 0, "Node index should be positive");
    
    std::cout << "  Node J1 has index: " << nodeIndex << std::endl;
    
    // Get pressure at J1 (EN_PRESSURE = 11)
    double pressure = 0.0;
    errorCode = EpanetWrapper::GetNodeValue(nodeIndex, 11, &pressure);
    TEST_ASSERT(errorCode == 0, "GetNodeValue should succeed for pressure");
    
    std::cout << "  Pressure at J1: " << pressure << " psi" << std::endl;
    
    // Get head at J1 (EN_HEAD = 10)
    double head = 0.0;
    errorCode = EpanetWrapper::GetNodeValue(nodeIndex, 10, &head);
    TEST_ASSERT(errorCode == 0, "GetNodeValue should succeed for head");
    
    std::cout << "  Head at J1: " << head << " ft" << std::endl;
    
    // Cleanup
    errorCode = EpanetWrapper::CloseHydraulics();
    TEST_ASSERT(errorCode == 0, "CloseHydraulics should succeed");
    
    errorCode = EpanetWrapper::Close();
    TEST_ASSERT(errorCode == 0, "Close should succeed");
    
    return true;
}

/**
 * Test 6: Error message mapping for hydraulic solver errors
 */
bool Test_ErrorMessages_HydraulicSolver() {
    // Test error code 103 (hydraulics not initialized)
    std::string msg103 = EpanetWrapper::GetErrorMessage(103);
    TEST_ASSERT(msg103.find("not initialized") != std::string::npos,
                "Error 103 should mention 'not initialized'");
    std::cout << "  Error 103: " << msg103 << std::endl;
    
    // Test error code 110 (cannot solve hydraulic equations)
    std::string msg110 = EpanetWrapper::GetErrorMessage(110);
    TEST_ASSERT(msg110.find("solve hydraulic") != std::string::npos,
                "Error 110 should mention 'solve hydraulic'");
    std::cout << "  Error 110: " << msg110 << std::endl;
    
    // Test error code 0 (success)
    std::string msg0 = EpanetWrapper::GetErrorMessage(0);
    TEST_ASSERT(msg0 == "Success", "Error 0 should return 'Success'");
    std::cout << "  Error 0: " << msg0 << std::endl;
    
    return true;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "EpanetWrapper Hydraulic Solver Tests" << std::endl;
    std::cout << "Task 4.2: Implement hydraulic solver methods" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // Run all tests
    RUN_TEST(Test_SolveHydraulics_NotInitialized);
    std::cout << std::endl;
    
    RUN_TEST(Test_NextHydraulicStep_NotInitialized);
    std::cout << std::endl;
    
    RUN_TEST(Test_HydraulicLifecycle_Complete);
    std::cout << std::endl;
    
    RUN_TEST(Test_MultipleTimesteps_Loop);
    std::cout << std::endl;
    
    RUN_TEST(Test_GetNodeValues_AfterSolve);
    std::cout << std::endl;
    
    RUN_TEST(Test_ErrorMessages_HydraulicSolver);
    std::cout << std::endl;
    
    // Print summary
    std::cout << "========================================" << std::endl;
    std::cout << "Test Summary" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Tests Passed: " << g_testsPassed << std::endl;
    std::cout << "Tests Failed: " << g_testsFailed << std::endl;
    std::cout << "========================================" << std::endl;
    
    return (g_testsFailed == 0) ? 0 : 1;
}
