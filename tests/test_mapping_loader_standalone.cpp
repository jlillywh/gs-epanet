/**
 * @file test_mapping_loader_standalone.cpp
 * @brief Standalone test for MappingLoader class
 * 
 * This test verifies that MappingLoader can:
 * - Load valid JSON configuration files
 * - Validate configuration schema
 * - Detect missing required fields
 * - Detect array length mismatches
 * - Detect invalid object types and properties
 * - Resolve element names to EPANET indices
 */

#include "../src/MappingLoader.h"
#include "../src/EpanetWrapper.h"
#include <iostream>
#include <string>

// Test result tracking
int g_tests_passed = 0;
int g_tests_failed = 0;

// Helper macro for test assertions
#define TEST_ASSERT(condition, message) \
    if (condition) { \
        std::cout << "[PASS] " << message << std::endl; \
        g_tests_passed++; \
    } else { \
        std::cout << "[FAIL] " << message << std::endl; \
        g_tests_failed++; \
    }

/**
 * @brief Test loading a valid configuration file
 */
void TestLoadValidConfig() {
    std::cout << "\n=== Test: Load Valid Config ===" << std::endl;
    
    MappingConfig config;
    std::string error;
    
    bool result = MappingLoader::LoadConfig("test_data/valid_config.json", config, error);
    
    TEST_ASSERT(result == true, "LoadConfig should succeed for valid file");
    TEST_ASSERT(config.version == "1.0", "Version should be '1.0'");
    TEST_ASSERT(config.logging_level == "INFO", "Logging level should be 'INFO'");
    TEST_ASSERT(config.inp_file == "test_model.inp", "INP file should be 'test_model.inp'");
    TEST_ASSERT(config.hydraulic_timestep == 300, "Hydraulic timestep should be 300");
    TEST_ASSERT(config.input_count == 3, "Input count should be 3");
    TEST_ASSERT(config.output_count == 4, "Output count should be 4");
    TEST_ASSERT(config.inputs.size() == 3, "Inputs array should have 3 elements");
    TEST_ASSERT(config.outputs.size() == 4, "Outputs array should have 4 elements");
    
    // Check first input (ElapsedTime)
    if (config.inputs.size() > 0) {
        TEST_ASSERT(config.inputs[0].index == 0, "Input[0] index should be 0");
        TEST_ASSERT(config.inputs[0].name == "ElapsedTime", "Input[0] name should be 'ElapsedTime'");
        TEST_ASSERT(config.inputs[0].object_type == "SYSTEM", "Input[0] object_type should be 'SYSTEM'");
        TEST_ASSERT(config.inputs[0].property == "ELAPSEDTIME", "Input[0] property should be 'ELAPSEDTIME'");
    }
    
    // Check first output
    if (config.outputs.size() > 0) {
        TEST_ASSERT(config.outputs[0].index == 0, "Output[0] index should be 0");
        TEST_ASSERT(config.outputs[0].name == "JUNCTION_1", "Output[0] name should be 'JUNCTION_1'");
        TEST_ASSERT(config.outputs[0].object_type == "NODE", "Output[0] object_type should be 'NODE'");
        TEST_ASSERT(config.outputs[0].property == "PRESSURE", "Output[0] property should be 'PRESSURE'");
    }
}

/**
 * @brief Test loading a missing configuration file
 */
void TestLoadMissingFile() {
    std::cout << "\n=== Test: Load Missing File ===" << std::endl;
    
    MappingConfig config;
    std::string error;
    
    bool result = MappingLoader::LoadConfig("nonexistent.json", config, error);
    
    TEST_ASSERT(result == false, "LoadConfig should fail for missing file");
    TEST_ASSERT(!error.empty(), "Error message should not be empty");
    TEST_ASSERT(error.find("not found") != std::string::npos, "Error should mention 'not found'");
    
    std::cout << "Error message: " << error << std::endl;
}

/**
 * @brief Test validating a valid configuration
 */
void TestValidateValidConfig() {
    std::cout << "\n=== Test: Validate Valid Config ===" << std::endl;
    
    MappingConfig config;
    std::string error;
    
    // Load valid config first
    MappingLoader::LoadConfig("test_data/valid_config.json", config, error);
    
    // Now validate it
    bool result = MappingLoader::ValidateConfig(config, error);
    
    TEST_ASSERT(result == true, "ValidateConfig should succeed for valid config");
    TEST_ASSERT(error.empty(), "Error message should be empty for valid config");
}

/**
 * @brief Test validating config with array length mismatch
 */
void TestValidateCountMismatch() {
    std::cout << "\n=== Test: Validate Count Mismatch ===" << std::endl;
    
    MappingConfig config;
    config.version = "1.0";
    config.logging_level = "INFO";
    config.input_count = 5;  // Mismatch: says 5 but array has 2
    config.output_count = 2;
    
    InputMapping input1;
    input1.index = 0;
    input1.name = "ElapsedTime";
    input1.object_type = "SYSTEM";
    input1.property = "ELAPSEDTIME";
    config.inputs.push_back(input1);
    
    InputMapping input2;
    input2.index = 1;
    input2.name = "JUNCTION_1";
    input2.object_type = "NODE";
    input2.property = "DEMAND";
    config.inputs.push_back(input2);
    
    OutputMapping output1;
    output1.index = 0;
    output1.name = "JUNCTION_1";
    output1.object_type = "NODE";
    output1.property = "PRESSURE";
    config.outputs.push_back(output1);
    
    OutputMapping output2;
    output2.index = 1;
    output2.name = "PIPE_1";
    output2.object_type = "LINK";
    output2.property = "FLOW";
    config.outputs.push_back(output2);
    
    std::string error;
    bool result = MappingLoader::ValidateConfig(config, error);
    
    TEST_ASSERT(result == false, "ValidateConfig should fail for count mismatch");
    TEST_ASSERT(!error.empty(), "Error message should not be empty");
    TEST_ASSERT(error.find("mismatch") != std::string::npos, "Error should mention 'mismatch'");
    
    std::cout << "Error message: " << error << std::endl;
}

/**
 * @brief Test validating config with invalid object type
 */
void TestValidateInvalidObjectType() {
    std::cout << "\n=== Test: Validate Invalid Object Type ===" << std::endl;
    
    MappingConfig config;
    config.version = "1.0";
    config.logging_level = "INFO";
    config.input_count = 1;
    config.output_count = 1;
    
    InputMapping input1;
    input1.index = 0;
    input1.name = "TEST";
    input1.object_type = "INVALID_TYPE";  // Invalid!
    input1.property = "DEMAND";
    config.inputs.push_back(input1);
    
    OutputMapping output1;
    output1.index = 0;
    output1.name = "JUNCTION_1";
    output1.object_type = "NODE";
    output1.property = "PRESSURE";
    config.outputs.push_back(output1);
    
    std::string error;
    bool result = MappingLoader::ValidateConfig(config, error);
    
    TEST_ASSERT(result == false, "ValidateConfig should fail for invalid object_type");
    TEST_ASSERT(!error.empty(), "Error message should not be empty");
    TEST_ASSERT(error.find("object_type") != std::string::npos, "Error should mention 'object_type'");
    
    std::cout << "Error message: " << error << std::endl;
}

/**
 * @brief Test validating config with invalid property
 */
void TestValidateInvalidProperty() {
    std::cout << "\n=== Test: Validate Invalid Property ===" << std::endl;
    
    MappingConfig config;
    config.version = "1.0";
    config.logging_level = "INFO";
    config.input_count = 1;
    config.output_count = 1;
    
    InputMapping input1;
    input1.index = 0;
    input1.name = "JUNCTION_1";
    input1.object_type = "NODE";
    input1.property = "INVALID_PROPERTY";  // Invalid for NODE!
    config.inputs.push_back(input1);
    
    OutputMapping output1;
    output1.index = 0;
    output1.name = "JUNCTION_1";
    output1.object_type = "NODE";
    output1.property = "PRESSURE";
    config.outputs.push_back(output1);
    
    std::string error;
    bool result = MappingLoader::ValidateConfig(config, error);
    
    TEST_ASSERT(result == false, "ValidateConfig should fail for invalid property");
    TEST_ASSERT(!error.empty(), "Error message should not be empty");
    TEST_ASSERT(error.find("property") != std::string::npos, "Error should mention 'property'");
    
    std::cout << "Error message: " << error << std::endl;
}

/**
 * @brief Test validating config with invalid logging level
 */
void TestValidateInvalidLoggingLevel() {
    std::cout << "\n=== Test: Validate Invalid Logging Level ===" << std::endl;
    
    MappingConfig config;
    config.version = "1.0";
    config.logging_level = "INVALID_LEVEL";  // Invalid!
    config.input_count = 1;
    config.output_count = 1;
    
    InputMapping input1;
    input1.index = 0;
    input1.name = "ElapsedTime";
    input1.object_type = "SYSTEM";
    input1.property = "ELAPSEDTIME";
    config.inputs.push_back(input1);
    
    OutputMapping output1;
    output1.index = 0;
    output1.name = "JUNCTION_1";
    output1.object_type = "NODE";
    output1.property = "PRESSURE";
    config.outputs.push_back(output1);
    
    std::string error;
    bool result = MappingLoader::ValidateConfig(config, error);
    
    TEST_ASSERT(result == false, "ValidateConfig should fail for invalid logging_level");
    TEST_ASSERT(!error.empty(), "Error message should not be empty");
    TEST_ASSERT(error.find("logging_level") != std::string::npos, "Error should mention 'logging_level'");
    
    std::cout << "Error message: " << error << std::endl;
}

/**
 * @brief Test resolving indices with valid element names
 */
void TestResolveIndicesValid() {
    std::cout << "\n=== Test: Resolve Indices Valid ===" << std::endl;
    
    // First, open an EPANET model
    int errorCode = EpanetWrapper::Open("test_data/simple_network.inp", "", "");
    TEST_ASSERT(errorCode == 0, "EPANET model should open successfully");
    
    if (errorCode == 0) {
        // Create a config with valid element names
        MappingConfig config;
        config.version = "1.0";
        config.logging_level = "INFO";
        config.input_count = 3;
        config.output_count = 3;
        
        // Add inputs
        InputMapping input1;
        input1.index = 0;
        input1.name = "ElapsedTime";
        input1.object_type = "SYSTEM";
        input1.property = "ELAPSEDTIME";
        input1.epanet_index = 0;
        config.inputs.push_back(input1);
        
        InputMapping input2;
        input2.index = 1;
        input2.name = "J1";  // Valid junction from simple_network.inp
        input2.object_type = "NODE";
        input2.property = "DEMAND";
        input2.epanet_index = 0;  // Not yet resolved
        config.inputs.push_back(input2);
        
        InputMapping input3;
        input3.index = 2;
        input3.name = "P1";  // Valid pipe from simple_network.inp
        input3.object_type = "LINK";
        input3.property = "STATUS";
        input3.epanet_index = 0;  // Not yet resolved
        config.inputs.push_back(input3);
        
        // Add outputs
        OutputMapping output1;
        output1.index = 0;
        output1.name = "J2";  // Valid junction
        output1.object_type = "NODE";
        output1.property = "PRESSURE";
        output1.epanet_index = 0;  // Not yet resolved
        config.outputs.push_back(output1);
        
        OutputMapping output2;
        output2.index = 1;
        output2.name = "T1";  // Valid tank
        output2.object_type = "NODE";
        output2.property = "TANKLEVEL";
        output2.epanet_index = 0;  // Not yet resolved
        config.outputs.push_back(output2);
        
        OutputMapping output3;
        output3.index = 2;
        output3.name = "P2";  // Valid pipe
        output3.object_type = "LINK";
        output3.property = "FLOW";
        output3.epanet_index = 0;  // Not yet resolved
        config.outputs.push_back(output3);
        
        // Resolve indices
        std::string error;
        bool result = MappingLoader::ResolveIndices(config, error);
        
        TEST_ASSERT(result == true, "ResolveIndices should succeed for valid element names");
        TEST_ASSERT(error.empty(), "Error message should be empty for successful resolution");
        
        // Check that indices were resolved (should be > 0)
        TEST_ASSERT(config.inputs[0].epanet_index == 0, "SYSTEM type should have index 0");
        TEST_ASSERT(config.inputs[1].epanet_index > 0, "J1 should have resolved index > 0");
        TEST_ASSERT(config.inputs[2].epanet_index > 0, "P1 should have resolved index > 0");
        TEST_ASSERT(config.outputs[0].epanet_index > 0, "J2 should have resolved index > 0");
        TEST_ASSERT(config.outputs[1].epanet_index > 0, "T1 should have resolved index > 0");
        TEST_ASSERT(config.outputs[2].epanet_index > 0, "P2 should have resolved index > 0");
        
        std::cout << "Resolved indices:" << std::endl;
        std::cout << "  J1 (NODE): " << config.inputs[1].epanet_index << std::endl;
        std::cout << "  P1 (LINK): " << config.inputs[2].epanet_index << std::endl;
        std::cout << "  J2 (NODE): " << config.outputs[0].epanet_index << std::endl;
        std::cout << "  T1 (NODE): " << config.outputs[1].epanet_index << std::endl;
        std::cout << "  P2 (LINK): " << config.outputs[2].epanet_index << std::endl;
        
        // Close EPANET
        EpanetWrapper::Close();
    }
}

/**
 * @brief Test resolving indices with invalid element name
 */
void TestResolveIndicesInvalid() {
    std::cout << "\n=== Test: Resolve Indices Invalid ===" << std::endl;
    
    // First, open an EPANET model
    int errorCode = EpanetWrapper::Open("test_data/simple_network.inp", "", "");
    TEST_ASSERT(errorCode == 0, "EPANET model should open successfully");
    
    if (errorCode == 0) {
        // Create a config with an invalid element name
        MappingConfig config;
        config.version = "1.0";
        config.logging_level = "INFO";
        config.input_count = 2;
        config.output_count = 1;
        
        // Add inputs
        InputMapping input1;
        input1.index = 0;
        input1.name = "ElapsedTime";
        input1.object_type = "SYSTEM";
        input1.property = "ELAPSEDTIME";
        input1.epanet_index = 0;
        config.inputs.push_back(input1);
        
        InputMapping input2;
        input2.index = 1;
        input2.name = "INVALID_NODE";  // Invalid node name!
        input2.object_type = "NODE";
        input2.property = "DEMAND";
        input2.epanet_index = 0;
        config.inputs.push_back(input2);
        
        // Add output
        OutputMapping output1;
        output1.index = 0;
        output1.name = "J1";
        output1.object_type = "NODE";
        output1.property = "PRESSURE";
        output1.epanet_index = 0;
        config.outputs.push_back(output1);
        
        // Try to resolve indices
        std::string error;
        bool result = MappingLoader::ResolveIndices(config, error);
        
        TEST_ASSERT(result == false, "ResolveIndices should fail for invalid element name");
        TEST_ASSERT(!error.empty(), "Error message should not be empty");
        TEST_ASSERT(error.find("INVALID_NODE") != std::string::npos, "Error should mention the invalid element name");
        TEST_ASSERT(error.find("not found") != std::string::npos, "Error should mention 'not found'");
        
        std::cout << "Error message: " << error << std::endl;
        
        // Close EPANET
        EpanetWrapper::Close();
    }
}

/**
 * @brief Test resolving indices with pre-resolved indices
 */
void TestResolveIndicesPreResolved() {
    std::cout << "\n=== Test: Resolve Indices Pre-Resolved ===" << std::endl;
    
    // First, open an EPANET model
    int errorCode = EpanetWrapper::Open("test_data/simple_network.inp", "", "");
    TEST_ASSERT(errorCode == 0, "EPANET model should open successfully");
    
    if (errorCode == 0) {
        // Create a config with pre-resolved indices
        MappingConfig config;
        config.version = "1.0";
        config.logging_level = "INFO";
        config.input_count = 1;
        config.output_count = 1;
        
        // Add input with pre-resolved index
        InputMapping input1;
        input1.index = 0;
        input1.name = "J1";
        input1.object_type = "NODE";
        input1.property = "DEMAND";
        input1.epanet_index = 99;  // Pre-resolved (arbitrary value)
        config.inputs.push_back(input1);
        
        // Add output with pre-resolved index
        OutputMapping output1;
        output1.index = 0;
        output1.name = "P1";
        output1.object_type = "LINK";
        output1.property = "FLOW";
        output1.epanet_index = 88;  // Pre-resolved (arbitrary value)
        config.outputs.push_back(output1);
        
        // Resolve indices (should skip pre-resolved ones)
        std::string error;
        bool result = MappingLoader::ResolveIndices(config, error);
        
        TEST_ASSERT(result == true, "ResolveIndices should succeed");
        TEST_ASSERT(error.empty(), "Error message should be empty");
        
        // Check that pre-resolved indices were not changed
        TEST_ASSERT(config.inputs[0].epanet_index == 99, "Pre-resolved input index should remain 99");
        TEST_ASSERT(config.outputs[0].epanet_index == 88, "Pre-resolved output index should remain 88");
        
        std::cout << "Pre-resolved indices preserved correctly" << std::endl;
        
        // Close EPANET
        EpanetWrapper::Close();
    }
}

/**
 * @brief Main test runner
 */
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "MappingLoader Standalone Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Run all tests
    TestLoadValidConfig();
    TestLoadMissingFile();
    TestValidateValidConfig();
    TestValidateCountMismatch();
    TestValidateInvalidObjectType();
    TestValidateInvalidProperty();
    TestValidateInvalidLoggingLevel();
    TestResolveIndicesValid();
    TestResolveIndicesInvalid();
    TestResolveIndicesPreResolved();
    
    // Print summary
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Summary" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Tests passed: " << g_tests_passed << std::endl;
    std::cout << "Tests failed: " << g_tests_failed << std::endl;
    std::cout << "Total tests:  " << (g_tests_passed + g_tests_failed) << std::endl;
    
    if (g_tests_failed == 0) {
        std::cout << "\nAll tests PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "\nSome tests FAILED!" << std::endl;
        return 1;
    }
}
