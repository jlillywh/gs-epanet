/**
 * @file EpanetBridge.cpp
 * @brief Main entry point for EPANET-GoldSim Bridge DLL
 * 
 * This file implements the GoldSim External Function API for coupling
 * GoldSim dynamic simulation software with the EPA EPANET hydraulic
 * modeling engine. The bridge manages the complete lifecycle of EPANET
 * simulations and provides real-time data exchange between GoldSim and EPANET.
 * 
 * Requirements: 1.1, 8.6
 */

#include "EpanetBridge.h"
#include "MappingLoader.h"
#include "EpanetWrapper.h"
#include "Logger.h"
#include "epanet2.h"
#include <string>
#include <vector>
#include <cstring>
#include <Windows.h>  // For ULONG_PTR type

// Undefine Windows macros that conflict with our enums
#ifdef ERROR
#undef ERROR
#endif

// ============================================================================
// Global Enumerations for GoldSim Interface
// ============================================================================

/**
 * @brief XFMethod enumeration for GoldSim External Function method IDs
 * 
 * These values are passed by GoldSim in the XFMethod parameter to indicate
 * which operation should be performed.
 */
enum XFMethodID {
    XF_INITIALIZE = 0,      ///< Initialize at start of each realization
    XF_CALCULATE = 1,       ///< Normal calculation with inputs/outputs
    XF_REP_VERSION = 2,     ///< Report DLL version
    XF_REP_ARGUMENTS = 3,   ///< Report input/output counts
    XF_CLEANUP = 99         ///< Cleanup and release resources
};

/**
 * @brief XFStatus enumeration for GoldSim External Function return status codes
 * 
 * These values are returned via the XFState parameter to indicate the
 * result of the operation.
 */
enum XFStatusID {
    XF_SUCCESS = 0,             ///< Success, continue simulation
    XF_FAILURE = 1,             ///< Fatal error, terminate simulation
    XF_CLEANUP_NOW = 99,        ///< Success, unload DLL immediately
    XF_FAILURE_WITH_MSG = -1,   ///< Fatal error with message pointer in outargs[0]
    XF_INCREASE_MEMORY = -2     ///< Need more memory, size in outargs[0]
};

// ============================================================================
// Static State Variables
// ============================================================================

/**
 * @brief Flag indicating whether the bridge has been initialized
 * 
 * Set to true during XF_INITIALIZE, false during XF_CLEANUP.
 * Used to validate that XF_CALCULATE is only called after initialization.
 */
static bool g_isInitialized = false;

/**
 * @brief Configuration loaded from JSON file
 * 
 * Contains all input/output mappings, logging settings, and metadata.
 */
static MappingConfig g_config;

/**
 * @brief Input mappings extracted from configuration
 * 
 * Cached copy of g_config.inputs for faster access during XF_CALCULATE.
 */
static std::vector<InputMapping> g_inputs;

/**
 * @brief Output mappings extracted from configuration
 * 
 * Cached copy of g_config.outputs for faster access during XF_CALCULATE.
 */
static std::vector<OutputMapping> g_outputs;

/**
 * @brief Logger instance for diagnostic output
 * 
 * Configured during XF_INITIALIZE based on logging_level in JSON.
 */
static Logger g_logger;

/**
 * @brief Flag indicating whether the log file has been opened
 * 
 * Set to true on first XF_INITIALIZE to ensure log file is only
 * truncated once per DLL load, not on every realization.
 */
static bool g_logFileOpened = false;

/**
 * @brief Static error buffer for error messages
 * 
 * Must be static to persist after function return when XFState = -1.
 * Size of 200 bytes provides sufficient space for descriptive error messages.
 * 
 * Requirements: 1.8, 8.6, 8.7
 */
static char g_errorBuffer[200];

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Set error message and return error status to GoldSim
 * 
 * This helper function copies an error message to the static error buffer,
 * ensures NULL termination, stores the buffer pointer in outargs[0], and
 * sets the status to XF_FAILURE_WITH_MSG.
 * 
 * The error buffer is static to ensure it persists after the function returns,
 * as required by the GoldSim External Function API.
 * 
 * Requirements: 1.8, 8.6, 8.7
 * 
 * @param message Error message to return to GoldSim
 * @param outargs Output array pointer (outargs[0] will receive error message pointer)
 * @param status Status pointer to set to XF_FAILURE_WITH_MSG
 */
static void SetErrorMessage(const std::string& message, double* outargs, int* status) {
    // Clear the error buffer
    memset(g_errorBuffer, 0, sizeof(g_errorBuffer));
    
    // Copy message to buffer, ensuring we don't overflow
    strncpy_s(g_errorBuffer, sizeof(g_errorBuffer), message.c_str(), sizeof(g_errorBuffer) - 1);
    
    // Ensure NULL termination (strncpy_s should do this, but be explicit)
    g_errorBuffer[sizeof(g_errorBuffer) - 1] = '\0';
    
    // Store pointer to error buffer in outargs[0]
    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
    *pAddr = (ULONG_PTR)g_errorBuffer;
    
    // Set status to indicate error with message
    *status = XF_FAILURE_WITH_MSG;
}

// ============================================================================
// Main External Function Entry Point
// ============================================================================

/**
 * @brief Main entry point for GoldSim External Function
 * 
 * This function implements the GoldSim External Function API and handles
 * all communication between GoldSim and EPANET. It is called by GoldSim
 * with different method IDs to perform initialization, calculation, reporting,
 * and cleanup operations.
 * 
 * Function Signature Requirements:
 * - Must use extern "C" linkage (no name mangling)
 * - Must use __declspec(dllexport) for visibility
 * - Function name is case-sensitive (must match exactly in GoldSim)
 * - Return type must be void (return value is ignored by GoldSim)
 * - All data exchange via double arrays only
 * 
 * Calling Sequence:
 * 1. Before simulation: XF_REP_VERSION → XF_REP_ARGUMENTS → XF_CLEANUP
 * 2. First calculation: Load DLL → XF_REP_VERSION → XF_REP_ARGUMENTS → XF_INITIALIZE → XF_CALCULATE
 * 3. Subsequent calculations: XF_CALCULATE (if inputs changed)
 * 4. Before each realization: XF_INITIALIZE (if DLL still loaded)
 * 5. After each realization: XF_CLEANUP (if "Cleanup After Realization" enabled)
 * 6. After simulation: XF_CLEANUP (if DLL still loaded)
 * 
 * Requirements: 1.1
 * 
 * @param methodID The operation to perform (XF_INITIALIZE, XF_CALCULATE, etc.)
 * @param status Pointer to status code to return (XF_SUCCESS, XF_FAILURE, etc.)
 * @param inargs Input array from GoldSim (read-only, must not be modified)
 * @param outargs Output array to GoldSim (write results here)
 */
extern "C" void __declspec(dllexport)
EpanetBridge(int methodID, int* status, double* inargs, double* outargs)
{
    // Default to success status
    *status = XF_SUCCESS;
    
    // Dispatch to appropriate handler based on method ID
    switch (methodID)
    {
        case XF_INITIALIZE:
            // Initialize at start of each realization
            // Load JSON configuration, open EPANET model, initialize hydraulics
            // Requirements: 1.2, 2.1, 3.1, 3.2, 7.1
            {
                std::string error;
                
                // Load JSON configuration
                if (!MappingLoader::LoadConfig("EpanetBridge.json", g_config, error)) {
                    SetErrorMessage(error, outargs, status);
                    break;
                }
                
                // Validate configuration
                if (!MappingLoader::ValidateConfig(g_config, error)) {
                    SetErrorMessage(error, outargs, status);
                    break;
                }
                
                // Initialize logger with configured log level
                LogLevel logLevel = LogLevel::INFO;
                if (g_config.logging_level == "OFF" || g_config.logging_level == "NONE") {
                    logLevel = LogLevel::OFF;
                } else if (g_config.logging_level == "ERROR") {
                    logLevel = LogLevel::ERROR;
                } else if (g_config.logging_level == "INFO") {
                    logLevel = LogLevel::INFO;
                } else if (g_config.logging_level == "DEBUG") {
                    logLevel = LogLevel::DEBUG;
                }
                g_logger.SetLevel(logLevel);
                
                // Open log file in append mode (preserves all calls)
                // To start fresh, manually delete epanet_bridge_debug.log before running
                if (!g_logFileOpened) {
                    g_logger.SetLogFile("epanet_bridge_debug.log", false);  // append mode
                    g_logFileOpened = true;
                    g_logger.Info("=== XF_INITIALIZE called - starting initialization ===");
                } else {
                    g_logger.Info("=== XF_INITIALIZE called again (subsequent initialization) ===");
                }
                
                g_logger.Info("XF_INITIALIZE called - starting initialization");
                g_logger.Info("Configuration version: " + g_config.version);
                g_logger.Info("EPANET model file: " + g_config.inp_file);
                g_logger.Info("Input count: " + std::to_string(g_config.input_count));
                g_logger.Info("Output count: " + std::to_string(g_config.output_count));
                
                // Open EPANET model
                int errcode = EpanetWrapper::Open(g_config.inp_file.c_str(), "", "");
                if (errcode != 0) {
                    std::string epanetError = EpanetWrapper::GetErrorMessage(errcode);
                    g_logger.Error("Failed to open EPANET model: " + epanetError);
                    std::string fullError = "Failed to open EPANET model '" + g_config.inp_file + "': " + epanetError;
                    SetErrorMessage(fullError, outargs, status);
                    break;
                }
                g_logger.Info("EPANET model opened successfully");
                
                // Resolve element indices
                if (!MappingLoader::ResolveIndices(g_config, error)) {
                    g_logger.Error("Failed to resolve element indices: " + error);
                    EpanetWrapper::Close();
                    SetErrorMessage(error, outargs, status);
                    break;
                }
                g_logger.Info("Element indices resolved successfully");
                
                // Open hydraulics system
                errcode = EpanetWrapper::OpenHydraulics();
                if (errcode != 0) {
                    std::string epanetError = EpanetWrapper::GetErrorMessage(errcode);
                    g_logger.Error("Failed to open hydraulics: " + epanetError);
                    EpanetWrapper::Close();
                    std::string fullError = "Failed to open hydraulics: " + epanetError;
                    SetErrorMessage(fullError, outargs, status);
                    break;
                }
                g_logger.Info("Hydraulics system opened successfully");
                
                // Initialize hydraulics
                errcode = EpanetWrapper::InitHydraulics(0);
                if (errcode != 0) {
                    std::string epanetError = EpanetWrapper::GetErrorMessage(errcode);
                    g_logger.Error("Failed to initialize hydraulics: " + epanetError);
                    EpanetWrapper::CloseHydraulics();
                    EpanetWrapper::Close();
                    std::string fullError = "Failed to initialize hydraulics: " + epanetError;
                    SetErrorMessage(fullError, outargs, status);
                    break;
                }
                g_logger.Info("Hydraulics initialized successfully");
                
                // Check if water quality simulation is enabled
                int qualType = 0;
                int traceNode = 0;
                errcode = ENgetqualtype(&qualType, &traceNode);
                if (errcode == 0 && qualType != 0) { // 0 = EN_NONE (no quality analysis)
                    g_logger.Info("Water quality simulation enabled (type: " + std::to_string(qualType) + ")");
                    
                    // Open water quality system
                    errcode = EpanetWrapper::OpenQuality();
                    if (errcode != 0) {
                        std::string epanetError = EpanetWrapper::GetErrorMessage(errcode);
                        g_logger.Error("Failed to open water quality: " + epanetError);
                        EpanetWrapper::CloseHydraulics();
                        EpanetWrapper::Close();
                        std::string fullError = "Failed to open water quality: " + epanetError;
                        SetErrorMessage(fullError, outargs, status);
                        break;
                    }
                    g_logger.Info("Water quality system opened successfully");
                } else {
                    g_logger.Info("Water quality simulation disabled");
                }
                
                // Cache input and output mappings for faster access during XF_CALCULATE
                g_inputs = g_config.inputs;
                g_outputs = g_config.outputs;
                
                // Mark as initialized
                g_isInitialized = true;
                g_logger.Info("Initialization complete - bridge ready for calculations");
                
                *status = XF_SUCCESS;
            }
            break;
            
        case XF_CALCULATE:
            // Normal calculation with inputs/outputs
            // Process inputs, solve hydraulics, extract outputs
            // Requirements: 1.3, 4.1-4.6, 5.1-5.8, 10.1-10.4
            {
                // Validate initialization
                if (!g_isInitialized) {
                    SetErrorMessage("XF_CALCULATE called before XF_INITIALIZE", outargs, status);
                    break;
                }
                
                // Extract ElapsedTime from inargs[0]
                double elapsedTime = inargs[0];
                g_logger.Debug("XF_CALCULATE called at ElapsedTime = " + std::to_string(elapsedTime));
                
                // Get current EPANET time before any operations
                long epanetTimeBefore = 0;
                ENgettimeparam(0, &epanetTimeBefore); // 0 = EN_DURATION, but we want current time
                // Actually, we need to track this through SolveHydraulics
                
                // Process all inputs (skip index 0 which is ElapsedTime)
                for (size_t i = 0; i < g_inputs.size(); i++) {
                    const InputMapping& input = g_inputs[i];
                    
                    // Skip ElapsedTime (SYSTEM/ELAPSEDTIME)
                    if (input.object_type == "SYSTEM") {
                        continue;
                    }
                    
                    double value = inargs[input.index];
                    g_logger.Debug("Processing input[" + std::to_string(input.index) + "]: " + 
                                  input.name + " = " + std::to_string(value));
                    
                    int errcode = 0;
                    
                    // Map object_type and property to EPANET API call
                    if (input.object_type == "NODE") {
                        if (input.property == "DEMAND") {
                            // EN_BASEDEMAND = 1
                            g_logger.Debug("Setting NODE " + input.name + " BASEDEMAND to " + std::to_string(value) + " GPM");
                            errcode = EpanetWrapper::SetNodeValue(input.epanet_index, 1, value);
                            
                            // Verify the value was set correctly
                            double verifyValue = 0.0;
                            int verifyErr = EpanetWrapper::GetNodeValue(input.epanet_index, 1, &verifyValue);
                            if (verifyErr == 0) {
                                g_logger.Debug("Verified NODE " + input.name + " BASEDEMAND = " + std::to_string(verifyValue) + " GPM");
                            }
                        } else if (input.property == "EMITTER") {
                            // EN_EMITTER = 3
                            g_logger.Debug("Setting NODE " + input.name + " EMITTER to " + std::to_string(value));
                            errcode = EpanetWrapper::SetNodeValue(input.epanet_index, 3, value);
                            
                            // Verify the value was set correctly
                            double verifyValue = 0.0;
                            int verifyErr = EpanetWrapper::GetNodeValue(input.epanet_index, 3, &verifyValue);
                            if (verifyErr == 0) {
                                g_logger.Debug("Verified NODE " + input.name + " EMITTER = " + std::to_string(verifyValue));
                            }
                        } else {
                            g_logger.Error("Unsupported NODE input property: " + input.property);
                            std::string error = "Unsupported NODE input property: " + input.property;
                            SetErrorMessage(error, outargs, status);
                            return;
                        }
                    } else if (input.object_type == "LINK") {
                        if (input.property == "STATUS") {
                            // EN_STATUS = 11
                            errcode = EpanetWrapper::SetLinkValue(input.epanet_index, 11, value);
                        } else if (input.property == "SETTING") {
                            // EN_SETTING = 12
                            // Validate range [0.0, 1.0] for pump speed/valve setting
                            if (value < 0.0 || value > 1.0) {
                                g_logger.Error("LINK " + input.name + " SETTING value " + 
                                             std::to_string(value) + " is out of range [0.0, 1.0]");
                            }
                            errcode = EpanetWrapper::SetLinkValue(input.epanet_index, 12, value);
                        } else {
                            g_logger.Error("Unsupported LINK input property: " + input.property);
                            std::string error = "Unsupported LINK input property: " + input.property;
                            SetErrorMessage(error, outargs, status);
                            return;
                        }
                    } else if (input.object_type == "PATTERN") {
                        if (input.property == "MULTIPLIER") {
                            // Pattern multiplier - need to determine period from elapsed time
                            // For now, use period 1 as a placeholder
                            // TODO: Calculate correct period based on elapsed time and pattern interval
                            errcode = EpanetWrapper::SetPatternValue(input.epanet_index, 1, value);
                        } else {
                            g_logger.Error("Unsupported PATTERN input property: " + input.property);
                            std::string error = "Unsupported PATTERN input property: " + input.property;
                            SetErrorMessage(error, outargs, status);
                            return;
                        }
                    }
                    
                    // Check for EPANET errors
                    if (errcode != 0) {
                        std::string epanetError = EpanetWrapper::GetErrorMessage(errcode);
                        g_logger.Error("Failed to set " + input.object_type + " " + input.name + 
                                      " " + input.property + ": " + epanetError);
                        std::string fullError = "Failed to set " + input.object_type + " " + 
                                              input.name + " " + input.property + ": " + epanetError;
                        SetErrorMessage(fullError, outargs, status);
                        return;
                    }
                }
                
                // Solve hydraulics - advance EPANET to match GoldSim's elapsed time
                g_logger.Debug("Solving hydraulics...");
                long currentTime = 0;
                long nextTime = 0;
                int errcode = 0;
                
                // Advance EPANET timesteps until we reach the target elapsed time
                // Note: We advance BEFORE solving (except at time 0)
                while (true) {
                    // Solve hydraulics at current EPANET time
                    errcode = EpanetWrapper::SolveHydraulics(&currentTime);
                    if (errcode > 100) {  // Error codes > 100 are fatal errors
                        std::string epanetError = EpanetWrapper::GetErrorMessage(errcode);
                        g_logger.Error("Hydraulic solver failed: " + epanetError);
                        std::string fullError = "EPANET hydraulic solver failed at time " + 
                                              std::to_string(currentTime) + ": " + epanetError;
                        SetErrorMessage(fullError, outargs, status);
                        return;
                    }
                    
                    g_logger.Debug("Hydraulics solved at EPANET time " + std::to_string(currentTime) + 
                                  " (target: " + std::to_string((long)elapsedTime) + ")");
                    
                    // Check if we've reached the target time
                    if (currentTime >= (long)elapsedTime) {
                        // We're at or past the target time, use these results
                        break;
                    }
                    
                    // We haven't reached target yet, advance to next hydraulic timestep
                    errcode = EpanetWrapper::NextHydraulicStep(&nextTime);
                    if (errcode != 0) {
                        std::string epanetError = EpanetWrapper::GetErrorMessage(errcode);
                        g_logger.Error("Failed to advance hydraulic timestep: " + epanetError);
                        std::string fullError = "Failed to advance EPANET timestep: " + epanetError;
                        SetErrorMessage(fullError, outargs, status);
                        return;
                    }
                    
                    // Check if simulation is complete
                    if (nextTime == 0) {
                        g_logger.Info("EPANET simulation complete at time " + std::to_string(currentTime));
                        break;
                    }
                }
                
                g_logger.Debug("Hydraulics solved successfully at time " + std::to_string(currentTime));
                
                // CRITICAL: Call ENnextH one more time to update tank volumes based on the flows
                // computed in the last ENrunH call. Tank levels are updated during ENnextH, not ENrunH.
                // Without this, tank levels will remain at their initial values.
                errcode = EpanetWrapper::NextHydraulicStep(&nextTime);
                if (errcode != 0 && errcode != 1) {  // Error code 1 means simulation complete, which is OK
                    std::string epanetError = EpanetWrapper::GetErrorMessage(errcode);
                    g_logger.Error("Failed to advance to next timestep for tank update: " + epanetError);
                    // Don't fail here - just log the warning
                    g_logger.Info("Continuing with current tank levels (may not reflect latest flows)");
                }
                g_logger.Debug("Tank volumes updated for time " + std::to_string(currentTime));
                
                // Solve water quality if enabled
                if (EpanetWrapper::IsQualityOpen()) {
                    g_logger.Debug("Solving water quality...");
                    long qualTime = 0;
                    int qualErrcode = EpanetWrapper::SolveQuality(&qualTime);
                    if (qualErrcode != 0) {
                        std::string epanetError = EpanetWrapper::GetErrorMessage(qualErrcode);
                        g_logger.Error("Water quality solver failed: " + epanetError);
                        std::string fullError = "EPANET water quality solver failed at time " + 
                                              std::to_string(elapsedTime) + ": " + epanetError;
                        SetErrorMessage(fullError, outargs, status);
                        return;
                    }
                    g_logger.Debug("Water quality solved successfully at time " + std::to_string(qualTime));
                }
                
                // Extract all outputs
                for (size_t i = 0; i < g_outputs.size(); i++) {
                    const OutputMapping& output = g_outputs[i];
                    double value = 0.0;
                    int errcode = 0;
                    
                    g_logger.Debug("Extracting output[" + std::to_string(output.index) + "]: " + 
                                  output.name + " " + output.property);
                    
                    // Map object_type and property to EPANET API call
                    if (output.object_type == "NODE") {
                        if (output.property == "PRESSURE") {
                            // EN_PRESSURE = 11
                            errcode = EpanetWrapper::GetNodeValue(output.epanet_index, 11, &value);
                        } else if (output.property == "HEAD") {
                            // EN_HEAD = 10
                            errcode = EpanetWrapper::GetNodeValue(output.epanet_index, 10, &value);
                        } else if (output.property == "DEMAND") {
                            // EN_DEMAND = 9
                            errcode = EpanetWrapper::GetNodeValue(output.epanet_index, 9, &value);
                        } else if (output.property == "TANKLEVEL") {
                            // EN_TANKLEVEL = 8
                            errcode = EpanetWrapper::GetNodeValue(output.epanet_index, 8, &value);
                        } else if (output.property == "QUALITY") {
                            // EN_QUALITY = 12
                            errcode = EpanetWrapper::GetNodeValue(output.epanet_index, 12, &value);
                        } else {
                            g_logger.Error("Unsupported NODE output property: " + output.property);
                            std::string error = "Unsupported NODE output property: " + output.property;
                            SetErrorMessage(error, outargs, status);
                            return;
                        }
                    } else if (output.object_type == "LINK") {
                        if (output.property == "FLOW") {
                            // EN_FLOW = 8
                            errcode = EpanetWrapper::GetLinkValue(output.epanet_index, 8, &value);
                        } else if (output.property == "VELOCITY") {
                            // EN_VELOCITY = 9
                            errcode = EpanetWrapper::GetLinkValue(output.epanet_index, 9, &value);
                        } else if (output.property == "HEADLOSS") {
                            // EN_HEADLOSS = 10
                            errcode = EpanetWrapper::GetLinkValue(output.epanet_index, 10, &value);
                        } else if (output.property == "STATUS") {
                            // EN_STATUS = 11
                            errcode = EpanetWrapper::GetLinkValue(output.epanet_index, 11, &value);
                        } else if (output.property == "SETTING") {
                            // EN_SETTING = 12
                            errcode = EpanetWrapper::GetLinkValue(output.epanet_index, 12, &value);
                        } else if (output.property == "QUALITY") {
                            // EN_LINKQUAL = 14
                            errcode = EpanetWrapper::GetLinkValue(output.epanet_index, 14, &value);
                        } else if (output.property == "POWER") {
                            // EN_PUMP_POWER = 18 (Pump constant power rating in horsepower)
                            g_logger.Debug("Getting LINK " + output.name + " POWER");
                            errcode = EpanetWrapper::GetLinkValue(output.epanet_index, 18, &value);
                        } else if (output.property == "EFFICIENCY") {
                            // EN_PUMP_EFFIC = 17 (Current computed pump efficiency in percent)
                            g_logger.Debug("Getting LINK " + output.name + " EFFICIENCY");
                            errcode = EpanetWrapper::GetLinkValue(output.epanet_index, 17, &value);
                        } else {
                            g_logger.Error("Unsupported LINK output property: " + output.property);
                            std::string error = "Unsupported LINK output property: " + output.property;
                            SetErrorMessage(error, outargs, status);
                            return;
                        }
                    }
                    
                    // Check for EPANET errors
                    if (errcode != 0) {
                        std::string epanetError = EpanetWrapper::GetErrorMessage(errcode);
                        g_logger.Error("Failed to get " + output.object_type + " " + output.name + 
                                      " " + output.property + ": " + epanetError);
                        std::string fullError = "Failed to get " + output.object_type + " " + 
                                              output.name + " " + output.property + ": " + epanetError;
                        SetErrorMessage(fullError, outargs, status);
                        return;
                    }
                    
                    // Store result in output array
                    outargs[output.index] = value;
                    g_logger.Debug("Output[" + std::to_string(output.index) + "] = " + std::to_string(value));
                }
                
                g_logger.Debug("XF_CALCULATE completed successfully");
                *status = XF_SUCCESS;
            }
            break;
            
        case XF_REP_VERSION:
            // Report DLL version number
            // Requirements: 1.4
            outargs[0] = 1.1;  // Version 1.1.0
            break;
            
        case XF_REP_ARGUMENTS:
            // Report input/output counts from JSON configuration
            // Requirements: 1.5
            {
                // Load JSON configuration if not already loaded
                // This allows GoldSim to query argument counts before initialization
                if (!g_isInitialized) {
                    std::string error;
                    if (!MappingLoader::LoadConfig("EpanetBridge.json", g_config, error)) {
                        // Failed to load configuration - return error
                        SetErrorMessage(error, outargs, status);
                        break;
                    }
                    
                    // Validate configuration
                    if (!MappingLoader::ValidateConfig(g_config, error)) {
                        // Configuration validation failed - return error
                        SetErrorMessage(error, outargs, status);
                        break;
                    }
                }
                
                // Return input and output counts from configuration
                outargs[0] = static_cast<double>(g_config.input_count);
                outargs[1] = static_cast<double>(g_config.output_count);
                *status = XF_SUCCESS;
            }
            break;
            
        case XF_CLEANUP:
            // Cleanup and release resources
            // Close EPANET, close log file, clear state
            // Requirements: 1.6, 3.5, 9.5
            {
                g_logger.Info("XF_CLEANUP called - starting cleanup");
                
                // Close water quality system if open
                if (EpanetWrapper::IsQualityOpen()) {
                    g_logger.Debug("Closing water quality system...");
                    int errcode = EpanetWrapper::CloseQuality();
                    if (errcode != 0) {
                        std::string epanetError = EpanetWrapper::GetErrorMessage(errcode);
                        g_logger.Error("Failed to close water quality: " + epanetError);
                        // Continue cleanup even if this fails
                    } else {
                        g_logger.Debug("Water quality system closed successfully");
                    }
                }
                
                // Close hydraulics system if open
                if (EpanetWrapper::IsHydraulicsOpen()) {
                    g_logger.Debug("Closing hydraulics system...");
                    int errcode = EpanetWrapper::CloseHydraulics();
                    if (errcode != 0) {
                        std::string epanetError = EpanetWrapper::GetErrorMessage(errcode);
                        g_logger.Error("Failed to close hydraulics: " + epanetError);
                        // Continue cleanup even if this fails
                    } else {
                        g_logger.Debug("Hydraulics system closed successfully");
                    }
                }
                
                // Close EPANET project if open
                if (EpanetWrapper::IsOpen()) {
                    g_logger.Debug("Closing EPANET project...");
                    int errcode = EpanetWrapper::Close();
                    if (errcode != 0) {
                        std::string epanetError = EpanetWrapper::GetErrorMessage(errcode);
                        g_logger.Error("Failed to close EPANET: " + epanetError);
                        // Continue cleanup even if this fails
                    } else {
                        g_logger.Debug("EPANET project closed successfully");
                    }
                }
                
                // Clear all static state variables
                g_inputs.clear();
                g_outputs.clear();
                g_config = MappingConfig();  // Reset to default
                
                // Mark as uninitialized
                g_isInitialized = false;
                
                g_logger.Info("Cleanup complete - bridge reset to uninitialized state");
                
                // Close log file (this should be last so we can log everything)
                g_logger.Close();
                
                *status = XF_SUCCESS;
            }
            break;
            
        default:
            // Unknown method ID - return failure
            *status = XF_FAILURE;
            strcpy_s(g_errorBuffer, sizeof(g_errorBuffer), "Unknown XFMethod ID");
            break;
    }
}
