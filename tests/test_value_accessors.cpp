/**
 * @file test_value_accessors.cpp
 * @brief Unit tests for EpanetWrapper node and link value accessor methods
 * 
 * Tests the implementation of GetNodeValue, SetNodeValue, GetLinkValue,
 * SetLinkValue, GetPatternValue, and SetPatternValue methods.
 * 
 * Validates Requirements: 4.2, 4.3, 4.4, 4.5, 5.1, 5.2, 5.3, 5.4, 5.5, 5.6
 */

#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

// Include the wrapper header
#include "../src/EpanetWrapper.h"

// EPANET property constants (from epanet2_enums.h)
#define EN_ELEVATION    0
#define EN_BASEDEMAND   1
#define EN_PATTERN      2
#define EN_EMITTER      3
#define EN_INITQUAL     4
#define EN_SOURCEQUAL   5
#define EN_SOURCEPAT    6
#define EN_SOURCETYPE   7
#define EN_TANKLEVEL    8
#define EN_DEMAND       9
#define EN_HEAD         10
#define EN_PRESSURE     11
#define EN_QUALITY      12

#define EN_DIAMETER     0
#define EN_LENGTH       1
#define EN_ROUGHNESS    2
#define EN_MINORLOSS    3
#define EN_INITSTATUS   4
#define EN_INITSETTING  5
#define EN_KBULK        6
#define EN_KWALL        7
#define EN_FLOW         8
#define EN_VELOCITY     9
#define EN_HEADLOSS     10
#define EN_STATUS       11
#define EN_SETTING      12
#define EN_ENERGY       13

// Test helper macros
#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "FAILED: " << message << std::endl; \
        return false; \
    }

#define TEST_ASSERT_EQUAL(expected, actual, message) \
    if ((expected) != (actual)) { \
        std::cerr << "FAILED: " << message << " (expected: " << (expected) << ", actual: " << (actual) << ")" << std::endl; \
        return false; \
    }

#define TEST_ASSERT_NEAR(expected, actual, tolerance, message) \
    if (std::abs((expected) - (actual)) > (tolerance)) { \
        std::cerr << "FAILED: " << message << " (expected: " << (expected) << ", actual: " << (actual) << ", diff: " << std::abs((expected) - (actual)) << ")" << std::endl; \
        return false; \
    }

// Test model path (relative to tests directory)
const std::string TEST_MODEL = "test_data/simple_network.inp";

/**
 * Test 1: GetNodeValue and SetNodeValue for base demand
 * Validates Requirements: 4.2, 5.3
 */
bool test_node_demand_accessors() {
    std::cout << "Test 1: Node demand accessors..." << std::endl;
    
    // Open the test model
    int errorCode = EpanetWrapper::Open(TEST_MODEL);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to open test model");
    
    // Get node index for a junction
    int nodeIndex;
    errorCode = EpanetWrapper::GetNodeIndex("J1", &nodeIndex);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get node index for J1");
    
    // Get initial base demand
    double initialDemand;
    errorCode = EpanetWrapper::GetNodeValue(nodeIndex, EN_BASEDEMAND, &initialDemand);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get initial base demand");
    
    // Set a new base demand
    double newDemand = 150.0;
    errorCode = EpanetWrapper::SetNodeValue(nodeIndex, EN_BASEDEMAND, newDemand);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to set base demand");
    
    // Verify the demand was set correctly
    double retrievedDemand;
    errorCode = EpanetWrapper::GetNodeValue(nodeIndex, EN_BASEDEMAND, &retrievedDemand);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get updated base demand");
    TEST_ASSERT_NEAR(newDemand, retrievedDemand, 0.01, "Base demand not set correctly");
    
    // Close the model
    EpanetWrapper::Close();
    
    std::cout << "  PASSED" << std::endl;
    return true;
}

/**
 * Test 2: GetNodeValue for pressure (computed value)
 * Validates Requirements: 5.1
 */
bool test_node_pressure_accessor() {
    std::cout << "Test 2: Node pressure accessor..." << std::endl;
    
    // Open and initialize hydraulics
    int errorCode = EpanetWrapper::Open(TEST_MODEL);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to open test model");
    
    errorCode = EpanetWrapper::OpenHydraulics();
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to open hydraulics");
    
    errorCode = EpanetWrapper::InitHydraulics();
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to initialize hydraulics");
    
    // Solve hydraulics
    long currentTime;
    errorCode = EpanetWrapper::SolveHydraulics(&currentTime);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to solve hydraulics");
    
    // Get node index
    int nodeIndex;
    errorCode = EpanetWrapper::GetNodeIndex("J1", &nodeIndex);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get node index");
    
    // Get pressure
    double pressure;
    errorCode = EpanetWrapper::GetNodeValue(nodeIndex, EN_PRESSURE, &pressure);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get pressure");
    
    // Pressure should be a reasonable value (not NaN or infinity)
    TEST_ASSERT(!std::isnan(pressure), "Pressure is NaN");
    TEST_ASSERT(!std::isinf(pressure), "Pressure is infinite");
    
    // Close
    EpanetWrapper::CloseHydraulics();
    EpanetWrapper::Close();
    
    std::cout << "  PASSED (pressure = " << pressure << ")" << std::endl;
    return true;
}

/**
 * Test 3: GetNodeValue for head (computed value)
 * Validates Requirements: 5.2
 */
bool test_node_head_accessor() {
    std::cout << "Test 3: Node head accessor..." << std::endl;
    
    // Open and initialize hydraulics
    int errorCode = EpanetWrapper::Open(TEST_MODEL);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to open test model");
    
    errorCode = EpanetWrapper::OpenHydraulics();
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to open hydraulics");
    
    errorCode = EpanetWrapper::InitHydraulics();
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to initialize hydraulics");
    
    // Solve hydraulics
    long currentTime;
    errorCode = EpanetWrapper::SolveHydraulics(&currentTime);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to solve hydraulics");
    
    // Get node index
    int nodeIndex;
    errorCode = EpanetWrapper::GetNodeIndex("J1", &nodeIndex);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get node index");
    
    // Get head
    double head;
    errorCode = EpanetWrapper::GetNodeValue(nodeIndex, EN_HEAD, &head);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get head");
    
    // Head should be a reasonable value
    TEST_ASSERT(!std::isnan(head), "Head is NaN");
    TEST_ASSERT(!std::isinf(head), "Head is infinite");
    TEST_ASSERT(head > 0, "Head should be positive");
    
    // Close
    EpanetWrapper::CloseHydraulics();
    EpanetWrapper::Close();
    
    std::cout << "  PASSED (head = " << head << ")" << std::endl;
    return true;
}

/**
 * Test 4: GetLinkValue and SetLinkValue for status
 * Validates Requirements: 4.3, 5.4
 */
bool test_link_status_accessors() {
    std::cout << "Test 4: Link status accessors..." << std::endl;
    
    // Open the test model
    int errorCode = EpanetWrapper::Open(TEST_MODEL);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to open test model");
    
    // Get link index for a pipe
    int linkIndex;
    errorCode = EpanetWrapper::GetLinkIndex("P1", &linkIndex);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get link index for P1");
    
    // Get initial status
    double initialStatus;
    errorCode = EpanetWrapper::GetLinkValue(linkIndex, EN_INITSTATUS, &initialStatus);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get initial status");
    
    // Set status to closed (0)
    errorCode = EpanetWrapper::SetLinkValue(linkIndex, EN_INITSTATUS, 0.0);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to set link status");
    
    // Verify status was set
    double retrievedStatus;
    errorCode = EpanetWrapper::GetLinkValue(linkIndex, EN_INITSTATUS, &retrievedStatus);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get updated status");
    TEST_ASSERT_NEAR(0.0, retrievedStatus, 0.01, "Link status not set correctly");
    
    // Set status to open (1)
    errorCode = EpanetWrapper::SetLinkValue(linkIndex, EN_INITSTATUS, 1.0);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to set link status to open");
    
    errorCode = EpanetWrapper::GetLinkValue(linkIndex, EN_INITSTATUS, &retrievedStatus);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get updated status");
    TEST_ASSERT_NEAR(1.0, retrievedStatus, 0.01, "Link status not set correctly");
    
    // Close the model
    EpanetWrapper::Close();
    
    std::cout << "  PASSED" << std::endl;
    return true;
}

/**
 * Test 5: GetLinkValue for flow (computed value)
 * Validates Requirements: 5.4
 */
bool test_link_flow_accessor() {
    std::cout << "Test 5: Link flow accessor..." << std::endl;
    
    // Open and initialize hydraulics
    int errorCode = EpanetWrapper::Open(TEST_MODEL);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to open test model");
    
    errorCode = EpanetWrapper::OpenHydraulics();
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to open hydraulics");
    
    errorCode = EpanetWrapper::InitHydraulics();
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to initialize hydraulics");
    
    // Solve hydraulics
    long currentTime;
    errorCode = EpanetWrapper::SolveHydraulics(&currentTime);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to solve hydraulics");
    
    // Get link index
    int linkIndex;
    errorCode = EpanetWrapper::GetLinkIndex("P1", &linkIndex);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get link index");
    
    // Get flow
    double flow;
    errorCode = EpanetWrapper::GetLinkValue(linkIndex, EN_FLOW, &flow);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get flow");
    
    // Flow should be a reasonable value
    TEST_ASSERT(!std::isnan(flow), "Flow is NaN");
    TEST_ASSERT(!std::isinf(flow), "Flow is infinite");
    
    // Close
    EpanetWrapper::CloseHydraulics();
    EpanetWrapper::Close();
    
    std::cout << "  PASSED (flow = " << flow << ")" << std::endl;
    return true;
}

/**
 * Test 6: GetLinkValue for velocity (computed value)
 * Validates Requirements: 5.5
 */
bool test_link_velocity_accessor() {
    std::cout << "Test 6: Link velocity accessor..." << std::endl;
    
    // Open and initialize hydraulics
    int errorCode = EpanetWrapper::Open(TEST_MODEL);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to open test model");
    
    errorCode = EpanetWrapper::OpenHydraulics();
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to open hydraulics");
    
    errorCode = EpanetWrapper::InitHydraulics();
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to initialize hydraulics");
    
    // Solve hydraulics
    long currentTime;
    errorCode = EpanetWrapper::SolveHydraulics(&currentTime);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to solve hydraulics");
    
    // Get link index
    int linkIndex;
    errorCode = EpanetWrapper::GetLinkIndex("P1", &linkIndex);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get link index");
    
    // Get velocity
    double velocity;
    errorCode = EpanetWrapper::GetLinkValue(linkIndex, EN_VELOCITY, &velocity);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get velocity");
    
    // Velocity should be a reasonable value
    TEST_ASSERT(!std::isnan(velocity), "Velocity is NaN");
    TEST_ASSERT(!std::isinf(velocity), "Velocity is infinite");
    TEST_ASSERT(velocity >= 0, "Velocity should be non-negative");
    
    // Close
    EpanetWrapper::CloseHydraulics();
    EpanetWrapper::Close();
    
    std::cout << "  PASSED (velocity = " << velocity << ")" << std::endl;
    return true;
}

/**
 * Test 7: GetPatternValue and SetPatternValue
 * Validates Requirements: 4.5
 */
bool test_pattern_accessors() {
    std::cout << "Test 7: Pattern accessors..." << std::endl;
    
    // Open the test model
    int errorCode = EpanetWrapper::Open(TEST_MODEL);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to open test model");
    
    // Pattern index 1 (first pattern in model)
    int patternIndex = 1;
    int period = 1;
    
    // Get initial pattern value
    double initialValue;
    errorCode = EpanetWrapper::GetPatternValue(patternIndex, period, &initialValue);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get initial pattern value");
    
    // Set a new pattern value
    double newValue = 1.5;
    errorCode = EpanetWrapper::SetPatternValue(patternIndex, period, newValue);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to set pattern value");
    
    // Verify the pattern value was set correctly
    double retrievedValue;
    errorCode = EpanetWrapper::GetPatternValue(patternIndex, period, &retrievedValue);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get updated pattern value");
    TEST_ASSERT_NEAR(newValue, retrievedValue, 0.01, "Pattern value not set correctly");
    
    // Close the model
    EpanetWrapper::Close();
    
    std::cout << "  PASSED" << std::endl;
    return true;
}

/**
 * Test 8: Float-to-double conversion accuracy
 * Validates that conversion between float (EPANET) and double (GoldSim) is accurate
 */
bool test_float_double_conversion() {
    std::cout << "Test 8: Float-to-double conversion accuracy..." << std::endl;
    
    // Open the test model
    int errorCode = EpanetWrapper::Open(TEST_MODEL);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to open test model");
    
    // Get node index
    int nodeIndex;
    errorCode = EpanetWrapper::GetNodeIndex("J1", &nodeIndex);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to get node index");
    
    // Test with various values that might lose precision in float conversion
    double testValues[] = {
        100.0,
        100.123456789,  // More precision than float can hold
        0.000001,       // Very small value
        999999.999,     // Large value with decimals
        -50.5,          // Negative value
        0.0             // Zero
    };
    
    for (double testValue : testValues) {
        // Set the value
        errorCode = EpanetWrapper::SetNodeValue(nodeIndex, EN_BASEDEMAND, testValue);
        TEST_ASSERT_EQUAL(0, errorCode, "Failed to set test value");
        
        // Get the value back
        double retrievedValue;
        errorCode = EpanetWrapper::GetNodeValue(nodeIndex, EN_BASEDEMAND, &retrievedValue);
        TEST_ASSERT_EQUAL(0, errorCode, "Failed to get test value");
        
        // Check that the value is within float precision tolerance
        // Float has ~7 decimal digits of precision
        double tolerance = std::abs(testValue) * 1e-6;
        if (tolerance < 1e-6) tolerance = 1e-6;
        
        TEST_ASSERT_NEAR(testValue, retrievedValue, tolerance, 
            "Float-to-double conversion lost precision");
    }
    
    // Close the model
    EpanetWrapper::Close();
    
    std::cout << "  PASSED" << std::endl;
    return true;
}

/**
 * Test 9: Error handling for invalid indices
 * Validates that accessor methods return appropriate error codes
 */
bool test_error_handling() {
    std::cout << "Test 9: Error handling for invalid indices..." << std::endl;
    
    // Open the test model
    int errorCode = EpanetWrapper::Open(TEST_MODEL);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to open test model");
    
    // Test with invalid node index
    double value;
    errorCode = EpanetWrapper::GetNodeValue(99999, EN_PRESSURE, &value);
    TEST_ASSERT(errorCode != 0, "Should return error for invalid node index");
    
    // Test with invalid link index
    errorCode = EpanetWrapper::GetLinkValue(99999, EN_FLOW, &value);
    TEST_ASSERT(errorCode != 0, "Should return error for invalid link index");
    
    // Test with invalid pattern index
    errorCode = EpanetWrapper::GetPatternValue(99999, 1, &value);
    TEST_ASSERT(errorCode != 0, "Should return error for invalid pattern index");
    
    // Test calling accessors before opening model
    EpanetWrapper::Close();
    
    errorCode = EpanetWrapper::GetNodeValue(1, EN_PRESSURE, &value);
    TEST_ASSERT_EQUAL(102, errorCode, "Should return 'No network data' error");
    
    errorCode = EpanetWrapper::SetNodeValue(1, EN_BASEDEMAND, 100.0);
    TEST_ASSERT_EQUAL(102, errorCode, "Should return 'No network data' error");
    
    std::cout << "  PASSED" << std::endl;
    return true;
}

/**
 * Test 10: Tank level accessor
 * Validates Requirements: 5.6
 */
bool test_tank_level_accessor() {
    std::cout << "Test 10: Tank level accessor..." << std::endl;
    
    // Open and initialize hydraulics
    int errorCode = EpanetWrapper::Open(TEST_MODEL);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to open test model");
    
    errorCode = EpanetWrapper::OpenHydraulics();
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to open hydraulics");
    
    errorCode = EpanetWrapper::InitHydraulics();
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to initialize hydraulics");
    
    // Solve hydraulics
    long currentTime;
    errorCode = EpanetWrapper::SolveHydraulics(&currentTime);
    TEST_ASSERT_EQUAL(0, errorCode, "Failed to solve hydraulics");
    
    // Get tank index (if model has a tank)
    int tankIndex;
    errorCode = EpanetWrapper::GetNodeIndex("T1", &tankIndex);
    
    if (errorCode == 0) {
        // Model has a tank, test tank level
        double tankLevel;
        errorCode = EpanetWrapper::GetNodeValue(tankIndex, EN_TANKLEVEL, &tankLevel);
        TEST_ASSERT_EQUAL(0, errorCode, "Failed to get tank level");
        
        // Tank level should be a reasonable value
        TEST_ASSERT(!std::isnan(tankLevel), "Tank level is NaN");
        TEST_ASSERT(!std::isinf(tankLevel), "Tank level is infinite");
        TEST_ASSERT(tankLevel >= 0, "Tank level should be non-negative");
        
        std::cout << "  PASSED (tank level = " << tankLevel << ")" << std::endl;
    } else {
        // Model doesn't have a tank, skip this test
        std::cout << "  SKIPPED (no tank in test model)" << std::endl;
    }
    
    // Close
    EpanetWrapper::CloseHydraulics();
    EpanetWrapper::Close();
    
    return true;
}

/**
 * Main test runner
 */
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "EpanetWrapper Value Accessors Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    int passed = 0;
    int failed = 0;
    
    // Run all tests
    if (test_node_demand_accessors()) passed++; else failed++;
    if (test_node_pressure_accessor()) passed++; else failed++;
    if (test_node_head_accessor()) passed++; else failed++;
    if (test_link_status_accessors()) passed++; else failed++;
    if (test_link_flow_accessor()) passed++; else failed++;
    if (test_link_velocity_accessor()) passed++; else failed++;
    if (test_pattern_accessors()) passed++; else failed++;
    if (test_float_double_conversion()) passed++; else failed++;
    if (test_error_handling()) passed++; else failed++;
    if (test_tank_level_accessor()) passed++; else failed++;
    
    // Print summary
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Test Summary:" << std::endl;
    std::cout << "  Passed: " << passed << std::endl;
    std::cout << "  Failed: " << failed << std::endl;
    std::cout << "========================================" << std::endl;
    
    return (failed == 0) ? 0 : 1;
}
