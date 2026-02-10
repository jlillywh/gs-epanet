/**
 * @file EpanetWrapper.h
 * @brief Wrapper for EPANET API with error handling and logging
 * 
 * This class provides a static interface to the EPANET 2.2 API with
 * comprehensive error handling and state management. All methods are
 * static to maintain a single EPANET project instance throughout the
 * bridge's lifecycle.
 */

#pragma once

#include <string>
#include <map>

/**
 * @class EpanetWrapper
 * @brief Static wrapper class for EPANET API functions
 * 
 * Provides lifecycle management, element index resolution, value accessors,
 * and error message mapping for the EPANET hydraulic modeling engine.
 * 
 * Typical usage sequence:
 * 1. Open() - Load EPANET model from .inp file
 * 2. OpenHydraulics() - Open hydraulic analysis system
 * 3. InitHydraulics() - Initialize hydraulic solver
 * 4. SolveHydraulics() - Solve for current timestep (repeated)
 * 5. NextHydraulicStep() - Advance to next timestep (repeated)
 * 6. CloseHydraulics() - Close hydraulic analysis system
 * 7. Close() - Close EPANET project and free memory
 */
class EpanetWrapper {
public:
    // ========================================================================
    // Lifecycle Management
    // ========================================================================
    
    /**
     * @brief Open an EPANET model from an input file
     * @param inpFile Path to EPANET .inp file
     * @param rptFile Path to report file (optional, empty string for default)
     * @param outFile Path to binary output file (optional, empty string for default)
     * @return EPANET error code (0 = success)
     * 
     * Loads the network model and prepares for hydraulic analysis.
     * Sets s_isOpen to true on success.
     */
    static int Open(const std::string& inpFile, 
                   const std::string& rptFile = "", 
                   const std::string& outFile = "");
    
    /**
     * @brief Open the hydraulic analysis system
     * @return EPANET error code (0 = success)
     * 
     * Must be called after Open() and before InitHydraulics().
     * Sets s_hydraulicsOpen to true on success.
     */
    static int OpenHydraulics();
    
    /**
     * @brief Initialize the hydraulic solver
     * @param initFlag Initialization flag (default: EN_SAVE_AND_INIT)
     * @return EPANET error code (0 = success)
     * 
     * Prepares the hydraulic solver for the first timestep.
     * Must be called after OpenHydraulics().
     */
    static int InitHydraulics(int initFlag = 11); // EN_SAVE_AND_INIT = 11
    
    /**
     * @brief Solve hydraulics for the current timestep
     * @param currentTime Pointer to receive current simulation time (seconds)
     * @return EPANET error code (0 = success)
     * 
     * Runs the hydraulic solver for the current timestep.
     * Results can be retrieved using GetNodeValue() and GetLinkValue().
     */
    static int SolveHydraulics(long* currentTime);
    
    /**
     * @brief Advance to the next hydraulic timestep
     * @param nextTime Pointer to receive time until next hydraulic event (seconds)
     * @return EPANET error code (0 = success)
     * 
     * Advances EPANET's internal clock to the next hydraulic timestep.
     * Should be called after SolveHydraulics().
     */
    static int NextHydraulicStep(long* nextTime);
    
    /**
     * @brief Close the hydraulic analysis system
     * @return EPANET error code (0 = success)
     * 
     * Releases hydraulic solver resources.
     * Sets s_hydraulicsOpen to false.
     */
    static int CloseHydraulics();
    
    /**
     * @brief Close the EPANET project and free all memory
     * @return EPANET error code (0 = success)
     * 
     * Closes all files and releases all EPANET resources.
     * Sets s_isOpen to false.
     */
    static int Close();
    
    // ========================================================================
    // Water Quality Management
    // ========================================================================
    
    /**
     * @brief Open the water quality analysis system
     * @return EPANET error code (0 = success)
     * 
     * Must be called after Open() and before SolveQuality().
     * Sets s_qualityOpen to true on success.
     * Requirements: 15.1, 15.5
     */
    static int OpenQuality();
    
    /**
     * @brief Solve water quality for the current timestep
     * @param currentTime Pointer to receive current simulation time (seconds)
     * @return EPANET error code (0 = success)
     * 
     * Runs the water quality solver for the current timestep.
     * Must be called after SolveHydraulics().
     * Results can be retrieved using GetNodeValue() and GetLinkValue() with QUALITY property.
     * Requirements: 15.2
     */
    static int SolveQuality(long* currentTime);
    
    /**
     * @brief Close the water quality analysis system
     * @return EPANET error code (0 = success)
     * 
     * Releases water quality solver resources.
     * Sets s_qualityOpen to false.
     * Requirements: 15.5
     */
    static int CloseQuality();
    
    // ========================================================================
    // Element Index Resolution
    // ========================================================================
    
    /**
     * @brief Get the index of a node by its ID name
     * @param name Node ID string
     * @param index Pointer to receive node index (1-based)
     * @return EPANET error code (0 = success, 240 = undefined node)
     * 
     * Resolves a node name to its internal EPANET index.
     * Used during configuration loading to map element names to indices.
     */
    static int GetNodeIndex(const std::string& name, int* index);
    
    /**
     * @brief Get the index of a link by its ID name
     * @param name Link ID string
     * @param index Pointer to receive link index (1-based)
     * @return EPANET error code (0 = success, 241 = undefined link)
     * 
     * Resolves a link name to its internal EPANET index.
     * Used during configuration loading to map element names to indices.
     */
    static int GetLinkIndex(const std::string& name, int* index);
    
    // ========================================================================
    // Node Operations
    // ========================================================================
    
    /**
     * @brief Get a node property value
     * @param index Node index (1-based)
     * @param property Property code (e.g., EN_PRESSURE, EN_DEMAND)
     * @param value Pointer to receive property value
     * @return EPANET error code (0 = success)
     * 
     * Retrieves computed or design values for a node.
     * See EN_NodeProperty enum in epanet2_enums.h for property codes.
     */
    static int GetNodeValue(int index, int property, double* value);
    
    /**
     * @brief Set a node property value
     * @param index Node index (1-based)
     * @param property Property code (e.g., EN_BASEDEMAND, EN_ELEVATION)
     * @param value New property value
     * @return EPANET error code (0 = success)
     * 
     * Modifies design parameters for a node.
     * See EN_NodeProperty enum in epanet2_enums.h for property codes.
     */
    static int SetNodeValue(int index, int property, double value);
    
    // ========================================================================
    // Link Operations
    // ========================================================================
    
    /**
     * @brief Get a link property value
     * @param index Link index (1-based)
     * @param property Property code (e.g., EN_FLOW, EN_VELOCITY)
     * @param value Pointer to receive property value
     * @return EPANET error code (0 = success)
     * 
     * Retrieves computed or design values for a link.
     * See EN_LinkProperty enum in epanet2_enums.h for property codes.
     */
    static int GetLinkValue(int index, int property, double* value);
    
    /**
     * @brief Set a link property value
     * @param index Link index (1-based)
     * @param property Property code (e.g., EN_STATUS, EN_SETTING)
     * @param value New property value
     * @return EPANET error code (0 = success)
     * 
     * Modifies design parameters or control settings for a link.
     * See EN_LinkProperty enum in epanet2_enums.h for property codes.
     */
    static int SetLinkValue(int index, int property, double value);
    
    // ========================================================================
    // Pattern Operations
    // ========================================================================
    
    /**
     * @brief Get a time pattern multiplier value
     * @param index Pattern index (1-based)
     * @param period Time period index (1-based)
     * @param value Pointer to receive pattern multiplier
     * @return EPANET error code (0 = success)
     * 
     * Retrieves the multiplier for a specific time period in a pattern.
     */
    static int GetPatternValue(int index, int period, double* value);
    
    /**
     * @brief Set a time pattern multiplier value
     * @param index Pattern index (1-based)
     * @param period Time period index (1-based)
     * @param value New pattern multiplier
     * @return EPANET error code (0 = success)
     * 
     * Modifies the multiplier for a specific time period in a pattern.
     */
    static int SetPatternValue(int index, int period, double value);
    
    // ========================================================================
    // Error Handling
    // ========================================================================
    
    /**
     * @brief Get a descriptive error message for an EPANET error code
     * @param errorCode EPANET error code
     * @return Human-readable error message string
     * 
     * Maps EPANET error codes to descriptive messages.
     * Returns "Unknown error code" for unrecognized codes.
     */
    static std::string GetErrorMessage(int errorCode);
    
    // ========================================================================
    // State Query
    // ========================================================================
    
    /**
     * @brief Check if an EPANET project is currently open
     * @return true if Open() has been called successfully
     */
    static bool IsOpen() { return s_isOpen; }
    
    /**
     * @brief Check if hydraulic analysis system is currently open
     * @return true if OpenHydraulics() has been called successfully
     */
    static bool IsHydraulicsOpen() { return s_hydraulicsOpen; }
    
    /**
     * @brief Check if water quality analysis system is currently open
     * @return true if OpenQuality() has been called successfully
     */
    static bool IsQualityOpen() { return s_qualityOpen; }

private:
    // ========================================================================
    // State Variables
    // ========================================================================
    
    /// Flag indicating if an EPANET project is currently open
    static bool s_isOpen;
    
    /// Flag indicating if hydraulic analysis system is currently open
    static bool s_hydraulicsOpen;
    
    /// Flag indicating if water quality analysis system is currently open
    static bool s_qualityOpen;
    
    // ========================================================================
    // Error Message Mapping
    // ========================================================================
    
    /**
     * @brief Initialize the error message map
     * @return Map of EPANET error codes to descriptive messages
     * 
     * Creates a static map of all known EPANET error codes.
     * Called once on first use of GetErrorMessage().
     */
    static std::map<int, std::string> InitializeErrorMessages();
    
    /// Static map of error codes to messages (initialized on first use)
    static const std::map<int, std::string> s_errorMessages;
};
