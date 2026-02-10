/**
 * @file EpanetWrapper.cpp
 * @brief Implementation of EPANET API wrapper
 * 
 * Provides a static interface to EPANET 2.2 with error handling,
 * state management, and comprehensive error message mapping.
 */

#include "EpanetWrapper.h"
#include "epanet2.h"

// ============================================================================
// Static Member Initialization
// ============================================================================

bool EpanetWrapper::s_isOpen = false;
bool EpanetWrapper::s_hydraulicsOpen = false;
bool EpanetWrapper::s_qualityOpen = false;

// Initialize error message map
const std::map<int, std::string> EpanetWrapper::s_errorMessages = 
    EpanetWrapper::InitializeErrorMessages();

// ============================================================================
// Lifecycle Management Implementation
// ============================================================================

int EpanetWrapper::Open(const std::string& inpFile, 
                        const std::string& rptFile, 
                        const std::string& outFile) {
    // Convert empty strings to empty C strings for EPANET API
    const char* rptPtr = rptFile.empty() ? "" : rptFile.c_str();
    const char* outPtr = outFile.empty() ? "" : outFile.c_str();
    
    int errorCode = ENopen(inpFile.c_str(), rptPtr, outPtr);
    
    if (errorCode == 0) {
        s_isOpen = true;
    }
    
    return errorCode;
}

int EpanetWrapper::OpenHydraulics() {
    if (!s_isOpen) {
        return 102; // No network data
    }
    
    int errorCode = ENopenH();
    
    if (errorCode == 0) {
        s_hydraulicsOpen = true;
    }
    
    return errorCode;
}

int EpanetWrapper::InitHydraulics(int initFlag) {
    if (!s_hydraulicsOpen) {
        return 103; // Hydraulics not initialized
    }
    
    return ENinitH(initFlag);
}

int EpanetWrapper::SolveHydraulics(long* currentTime) {
    if (!s_hydraulicsOpen) {
        return 103; // Hydraulics not initialized
    }
    
    return ENrunH(currentTime);
}

int EpanetWrapper::NextHydraulicStep(long* nextTime) {
    if (!s_hydraulicsOpen) {
        return 103; // Hydraulics not initialized
    }
    
    return ENnextH(nextTime);
}

int EpanetWrapper::CloseHydraulics() {
    if (!s_hydraulicsOpen) {
        return 0; // Already closed, not an error
    }
    
    int errorCode = ENcloseH();
    
    if (errorCode == 0) {
        s_hydraulicsOpen = false;
    }
    
    return errorCode;
}

int EpanetWrapper::Close() {
    // Close quality first if still open
    if (s_qualityOpen) {
        CloseQuality();
    }
    
    // Close hydraulics first if still open
    if (s_hydraulicsOpen) {
        CloseHydraulics();
    }
    
    if (!s_isOpen) {
        return 0; // Already closed, not an error
    }
    
    int errorCode = ENclose();
    
    if (errorCode == 0) {
        s_isOpen = false;
    }
    
    return errorCode;
}

// ============================================================================
// Water Quality Management Implementation
// ============================================================================

int EpanetWrapper::OpenQuality() {
    if (!s_isOpen) {
        return 102; // No network data
    }
    
    int errorCode = ENopenQ();
    
    if (errorCode == 0) {
        s_qualityOpen = true;
    }
    
    return errorCode;
}

int EpanetWrapper::SolveQuality(long* currentTime) {
    if (!s_qualityOpen) {
        return 105; // No water quality results available
    }
    
    return ENrunQ(currentTime);
}

int EpanetWrapper::CloseQuality() {
    if (!s_qualityOpen) {
        return 0; // Already closed, not an error
    }
    
    int errorCode = ENcloseQ();
    
    if (errorCode == 0) {
        s_qualityOpen = false;
    }
    
    return errorCode;
}

// ============================================================================
// Element Index Resolution Implementation
// ============================================================================

int EpanetWrapper::GetNodeIndex(const std::string& name, int* index) {
    if (!s_isOpen) {
        return 102; // No network data
    }
    
    // EPANET expects non-const char*, but doesn't modify it
    // We need to create a mutable copy
    // EN_MAXID = 31 from epanet2_enums.h
    char nameCopy[32]; // EN_MAXID + 1
    strncpy_s(nameCopy, sizeof(nameCopy), name.c_str(), 31);
    nameCopy[31] = '\0';
    
    return ENgetnodeindex(nameCopy, index);
}

int EpanetWrapper::GetLinkIndex(const std::string& name, int* index) {
    if (!s_isOpen) {
        return 102; // No network data
    }
    
    // EPANET expects non-const char*, but doesn't modify it
    // We need to create a mutable copy
    // EN_MAXID = 31 from epanet2_enums.h
    char nameCopy[32]; // EN_MAXID + 1
    strncpy_s(nameCopy, sizeof(nameCopy), name.c_str(), 31);
    nameCopy[31] = '\0';
    
    return ENgetlinkindex(nameCopy, index);
}

// ============================================================================
// Node Operations Implementation
// ============================================================================

int EpanetWrapper::GetNodeValue(int index, int property, double* value) {
    if (!s_isOpen) {
        return 102; // No network data
    }
    
    // EPANET 2.2 uses EN_API_FLOAT_TYPE which defaults to float
    // We need to handle the conversion
    float floatValue;
    int errorCode = ENgetnodevalue(index, property, &floatValue);
    
    if (errorCode == 0) {
        *value = static_cast<double>(floatValue);
    }
    
    return errorCode;
}

int EpanetWrapper::SetNodeValue(int index, int property, double value) {
    if (!s_isOpen) {
        return 102; // No network data
    }
    
    // Convert double to float for EPANET API
    float floatValue = static_cast<float>(value);
    
    return ENsetnodevalue(index, property, floatValue);
}

// ============================================================================
// Link Operations Implementation
// ============================================================================

int EpanetWrapper::GetLinkValue(int index, int property, double* value) {
    if (!s_isOpen) {
        return 102; // No network data
    }
    
    // EPANET 2.2 uses EN_API_FLOAT_TYPE which defaults to float
    // We need to handle the conversion
    float floatValue;
    int errorCode = ENgetlinkvalue(index, property, &floatValue);
    
    if (errorCode == 0) {
        *value = static_cast<double>(floatValue);
    }
    
    return errorCode;
}

int EpanetWrapper::SetLinkValue(int index, int property, double value) {
    if (!s_isOpen) {
        return 102; // No network data
    }
    
    // Convert double to float for EPANET API
    float floatValue = static_cast<float>(value);
    
    return ENsetlinkvalue(index, property, floatValue);
}

// ============================================================================
// Pattern Operations Implementation
// ============================================================================

int EpanetWrapper::GetPatternValue(int index, int period, double* value) {
    if (!s_isOpen) {
        return 102; // No network data
    }
    
    // EPANET 2.2 uses EN_API_FLOAT_TYPE which defaults to float
    // We need to handle the conversion
    float floatValue;
    int errorCode = ENgetpatternvalue(index, period, &floatValue);
    
    if (errorCode == 0) {
        *value = static_cast<double>(floatValue);
    }
    
    return errorCode;
}

int EpanetWrapper::SetPatternValue(int index, int period, double value) {
    if (!s_isOpen) {
        return 102; // No network data
    }
    
    // Convert double to float for EPANET API
    float floatValue = static_cast<float>(value);
    
    return ENsetpatternvalue(index, period, floatValue);
}

// ============================================================================
// Error Handling Implementation
// ============================================================================

std::string EpanetWrapper::GetErrorMessage(int errorCode) {
    // Return success message for code 0
    if (errorCode == 0) {
        return "Success";
    }
    
    // Look up error code in map
    auto it = s_errorMessages.find(errorCode);
    if (it != s_errorMessages.end()) {
        return it->second;
    }
    
    // Unknown error code
    return "Unknown error code: " + std::to_string(errorCode);
}

std::map<int, std::string> EpanetWrapper::InitializeErrorMessages() {
    std::map<int, std::string> messages;
    
    // Success
    messages[0] = "Success";
    
    // System errors (100-109)
    messages[101] = "Insufficient memory";
    messages[102] = "No network data available";
    messages[103] = "Hydraulics not initialized";
    messages[104] = "No hydraulics results available";
    messages[105] = "No water quality results available";
    messages[106] = "No results to report on";
    messages[107] = "Hydraulics supplied from external file";
    messages[108] = "Cannot use external file while hydraulics solver is active";
    messages[109] = "Cannot change time parameter when solver is active";
    
    // Solver errors (110-119)
    messages[110] = "Cannot solve hydraulic equations - system may be unstable";
    messages[111] = "Cannot solve water quality transport equations";
    messages[112] = "Cannot solve water quality equilibrium equations";
    messages[120] = "Cannot solve water quality equations";
    
    // Input file errors (200-209)
    messages[200] = "One or more errors in input file";
    messages[201] = "Syntax error in input file";
    messages[202] = "Cannot open input file";
    messages[203] = "Cannot open report file";
    messages[204] = "Cannot open binary output file";
    messages[205] = "Cannot open hydraulics file";
    messages[206] = "Hydraulics file does not match network data";
    messages[207] = "Cannot read hydraulics file";
    messages[208] = "Cannot save results to file";
    messages[209] = "Cannot save results to report file";
    
    // Input data errors (210-249)
    messages[210] = "Invalid format (too few fields)";
    messages[211] = "Invalid format (too many fields)";
    messages[212] = "Duplicate ID label";
    messages[213] = "Undefined node in link data";
    messages[214] = "Undefined node in control data";
    messages[215] = "Undefined link in control data";
    messages[216] = "Undefined pattern in demand data";
    messages[217] = "Undefined curve in pump data";
    messages[218] = "Invalid pump curve";
    messages[219] = "Invalid valve data";
    messages[220] = "Invalid pipe data";
    messages[221] = "Invalid node data";
    messages[222] = "Invalid link data";
    messages[223] = "Invalid curve data";
    messages[224] = "Invalid pattern data";
    messages[225] = "Invalid time parameter";
    messages[226] = "Invalid option parameter";
    messages[227] = "Invalid quality parameter";
    messages[228] = "Invalid source quality parameter";
    messages[229] = "Invalid reaction coefficient";
    messages[230] = "Invalid energy parameter";
    messages[231] = "Invalid hydraulic option";
    messages[232] = "Invalid quality option";
    messages[233] = "Invalid rule syntax";
    messages[234] = "Invalid rule clause";
    messages[235] = "Invalid rule priority";
    messages[236] = "Invalid control statement";
    messages[237] = "Invalid mixing model";
    messages[238] = "Invalid tank volume curve";
    messages[239] = "Invalid demand model";
    
    // Runtime errors (240-259)
    messages[240] = "Undefined node - node ID not found in network";
    messages[241] = "Undefined link - link ID not found in network";
    messages[242] = "Undefined time pattern";
    messages[243] = "Undefined curve";
    messages[244] = "Undefined simple control";
    messages[245] = "Undefined rule-based control";
    messages[246] = "Invalid node property code";
    messages[247] = "Invalid link property code";
    messages[248] = "Invalid time parameter code";
    messages[249] = "Invalid option parameter code";
    messages[250] = "Invalid property code";
    messages[251] = "Invalid curve type";
    messages[252] = "Invalid demand category";
    messages[253] = "Invalid node type";
    messages[254] = "Invalid link type";
    messages[255] = "Invalid pump type";
    messages[256] = "Invalid action code";
    messages[257] = "Invalid analysis statistic";
    messages[258] = "Invalid time parameter";
    messages[259] = "Invalid object type";
    
    // Additional errors (260+)
    messages[260] = "Undefined object";
    messages[261] = "Illegal object index";
    messages[262] = "Undefined demand category";
    messages[263] = "Invalid demand category";
    messages[264] = "Invalid curve index";
    messages[265] = "Invalid pattern index";
    messages[266] = "Invalid time period";
    
    return messages;
}
