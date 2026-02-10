# GS-EPANET Bridge - Technical Design Document

**Version**: 1.1.0  
**Last Updated**: February 9, 2026  
**Purpose**: Complete technical specification for rebuilding the EPANET-GoldSim Bridge

---

## 1. Project Overview

### 1.1 Purpose
GS-EPANET is a Windows DLL that enables timestep-by-timestep coupling between GoldSim simulation software and EPA EPANET hydraulic modeling engine. It allows GoldSim to control EPANET parameters (pump speeds, valve settings, demands) and retrieve results (pressures, flows, tank levels) at each simulation timestep.

### 1.2 Architecture Pattern
Based on the GS-SWMM bridge pattern:
- JSON-based configuration for input/output mapping
- GoldSim External Function API implementation
- Lifecycle management: Open → Set → Solve → Get → Close
- Configurable logging with multiple levels
- Example-driven documentation

### 1.3 Key Requirements
- Windows 10/11 (64-bit) target platform
- Visual Studio 2022+ with MSBuild 17.0+
- EPANET 2.2+ library integration
- Zero-copy data exchange via double arrays
- Thread-safe static state management
- Comprehensive error handling with descriptive messages

---

## 2. System Architecture

### 2.1 Component Overview
```
┌─────────────────────────────────────────────────────────────┐
│                        GoldSim                              │
│  (Calls EpanetBridge function with XFMethod, inargs, outargs)│
└────────────────────┬────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────┐
│                   gs_epanet.dll                             │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  EpanetBridge.cpp (Main Entry Point)                 │  │
│  │  - XFMethod dispatcher                               │  │
│  │  - State management                                  │  │
│  │  - Error handling                                    │  │
│  └──────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  MappingLoader.cpp (Configuration Parser)            │  │
│  │  - JSON parsing                                      │  │
│  │  - Schema validation                                 │  │
│  │  - Index resolution                                  │  │
│  └──────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  EpanetWrapper.cpp (EPANET API Wrapper)              │  │
│  │  - Function call wrappers                            │  │
│  │  - Error code translation                            │  │
│  │  - State tracking                                    │  │
│  └──────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  Logger.cpp (Diagnostic Logging)                     │  │
│  │  - File output                                       │  │
│  │  - Level filtering                                   │  │
│  │  - Timestamping                                      │  │
│  └──────────────────────────────────────────────────────┘  │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────┐
│                   epanet2.dll                               │
│  (EPA EPANET 2.2+ hydraulic/quality engine)                │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 File Structure
```
gs_epanet/
├── src/
│   ├── EpanetBridge.cpp/h      # Main GoldSim interface
│   ├── EpanetWrapper.cpp/h     # EPANET API wrapper
│   ├── MappingLoader.cpp/h     # JSON configuration parser
│   ├── Logger.cpp/h            # Logging utilities
│   └── EpanetBridge.vcxproj    # Visual Studio project
├── include/
│   ├── epanet2.h               # EPANET API header
│   ├── epanet2_enums.h         # EPANET constants
│   └── json.hpp                # nlohmann/json library
├── lib/
│   ├── epanet2.dll             # EPANET engine (x64)
│   └── epanet2.lib             # Import library
├── examples/
│   ├── Pump_to_Tank/           # Working example with GoldSim model
│   ├── Example1_Simple/        # Basic junction/pipe network
│   ├── Example2_MultiJunction/ # Multiple junctions
│   └── Example3_ValveControl/  # Valve control example
├── tests/
│   ├── test_*.cpp              # Unit/integration tests
│   ├── run_*.ps1               # Test runners
│   └── test_data/              # Test configurations
├── bin/
│   └── x64/                    # Build output (gs_epanet.dll)
├── build.ps1                   # Build automation script
├── create_distribution.ps1     # Package creation script
└── README.md                   # User documentation
```

---

## 3. GoldSim External Function API

### 3.1 Function Signature
```cpp
extern "C" void __declspec(dllexport)
EpanetBridge(int methodID, int* status, double* inargs, double* outargs);
```

**Critical Requirements**:
- `extern "C"` linkage (no name mangling)
- `__declspec(dllexport)` for DLL visibility
- Function name is case-sensitive
- Return type must be `void` (return value ignored by GoldSim)
- All data exchange via double arrays only

### 3.2 XFMethod Values

| methodID | Name | Purpose | When Called |
|----------|------|---------|-------------|
| 0 | XF_INITIALIZE | Load model, initialize state | Start of each realization |
| 1 | XF_CALCULATE | Process inputs, solve, return outputs | Each timestep |
| 2 | XF_REP_VERSION | Report DLL version | Before simulation |
| 3 | XF_REP_ARGUMENTS | Report input/output counts | Before simulation |
| 99 | XF_CLEANUP | Release resources, close files | End of realization |

### 3.3 XFStatus Return Values

| status | Name | Meaning | Action |
|--------|------|---------|--------|
| 0 | XF_SUCCESS | Success, continue | Normal operation |
| 1 | XF_FAILURE | Fatal error, terminate | Stop simulation |
| 99 | XF_CLEANUP_NOW | Success, unload DLL | Immediate unload |
| -1 | XF_FAILURE_WITH_MSG | Error with message | outargs[0] = error string pointer |
| -2 | XF_INCREASE_MEMORY | Need more memory | outargs[0] = required size |

### 3.4 Calling Sequence
```
Before Simulation:
  XF_REP_VERSION → XF_REP_ARGUMENTS → XF_CLEANUP

First Calculation:
  Load DLL → XF_REP_VERSION → XF_REP_ARGUMENTS → XF_INITIALIZE → XF_CALCULATE

Subsequent Calculations:
  XF_CALCULATE (if inputs changed)

Before Each Realization:
  XF_INITIALIZE (if DLL still loaded)

After Each Realization:
  XF_CLEANUP (if "Cleanup After Realization" enabled)

After Simulation:
  XF_CLEANUP (if DLL still loaded)
```

### 3.5 Input/Output Array Structure

**Input Array (inargs)**:
- `inargs[0]` - Always ElapsedTime (GoldSim's ETime in seconds)
- `inargs[1..N]` - User-defined inputs from JSON configuration
- Array is READ-ONLY (never modify)

**Output Array (outargs)**:
- `outargs[0..M]` - Results based on JSON configuration
- Special cases:
  - If status = -1: `outargs[0]` = pointer to static error string
  - If status = -2: `outargs[0]` = required memory size in doubles

---

## 4. JSON Configuration Schema

### 4.1 File Format (EpanetBridge.json)

```json
{
  "version": "1.1",
  "logging_level": "DEBUG",
  "inp_file": "model.inp",
  "inp_file_hash": "optional_md5_hash",
  "hydraulic_timestep": 300,
  "input_count": 2,
  "output_count": 3,
  "inputs": [
    {
      "index": 0,
      "name": "ElapsedTime",
      "object_type": "SYSTEM",
      "property": "ELAPSEDTIME"
    },
    {
      "index": 1,
      "name": "Pump1",
      "object_type": "LINK",
      "property": "SETTING",
      "epanet_index": 5,
      "comment": "Optional description"
    }
  ],
  "outputs": [
    {
      "index": 0,
      "name": "Tank1",
      "object_type": "NODE",
      "property": "HEAD",
      "epanet_index": 3
    }
  ]
}
```

### 4.2 Required Fields

| Field | Type | Description | Validation |
|-------|------|-------------|------------|
| version | string | Configuration version | Must be present |
| logging_level | string | OFF, ERROR, INFO, DEBUG | Optional, defaults to INFO |
| inp_file | string | Path to EPANET .inp file | Must exist |
| hydraulic_timestep | integer | EPANET timestep in seconds | Optional |
| input_count | integer | Number of inputs | Must match inputs array length |
| output_count | integer | Number of outputs | Must match outputs array length |
| inputs | array | Input mappings | Required |
| outputs | array | Output mappings | Required |

### 4.3 Input Mapping Fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| index | integer | Yes | Position in inargs array (0-based) |
| name | string | Yes | Element name from .inp file |
| object_type | string | Yes | SYSTEM, NODE, LINK, or PATTERN |
| property | string | Yes | Property to set (see section 5) |
| epanet_index | integer | No | Pre-resolved index (auto-resolved if omitted) |

### 4.4 Output Mapping Fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| index | integer | Yes | Position in outargs array (0-based) |
| name | string | Yes | Element name from .inp file |
| object_type | string | Yes | NODE or LINK |
| property | string | Yes | Property to get (see section 5) |
| epanet_index | integer | No | Pre-resolved index (auto-resolved if omitted) |

---

## 5. Supported Properties

### 5.1 Input Properties (Controllable)

**SYSTEM Properties**:
- `ELAPSEDTIME` - Simulation elapsed time (always input[0])

**NODE Properties**:
- `DEMAND` - Junction base demand (GPM/LPS)
- `EMITTER` - Emitter coefficient (GPM/psi^0.5)

**LINK Properties**:
- `STATUS` - Pipe/valve open (1) or closed (0)
- `SETTING` - Pump speed or valve position (0.0-1.0)

**PATTERN Properties**:
- `MULTIPLIER` - Pattern multiplier value

### 5.2 Output Properties (Monitored)

**NODE Properties**:

- `PRESSURE` - Pressure at junction (psi)
- `HEAD` - Hydraulic head (elevation + pressure, ft)
- `DEMAND` - Actual computed demand (GPM/LPS)
- `TANKLEVEL` - Tank water level parameter (use HEAD for computed level)
- `QUALITY` - Water quality concentration

**LINK Properties**:
- `FLOW` - Flow rate (GPM/LPS)
- `VELOCITY` - Flow velocity (ft/s)
- `HEADLOSS` - Head loss across element (ft)
- `STATUS` - Current status (0=closed, 1=open)
- `SETTING` - Current pump/valve setting
- `QUALITY` - Link water quality
- `POWER` - Pump power consumption (HP, computed)
- `EFFICIENCY` - Pump efficiency (fraction 0-1)

### 5.3 EPANET Property Codes

**Node Properties (EN_NodeProperty)**:
```cpp
EN_ELEVATION    = 0   // Elevation
EN_BASEDEMAND   = 1   // Primary demand baseline
EN_EMITTER      = 3   // Emitter coefficient
EN_TANKLEVEL    = 8   // Tank level parameter (read only)
EN_DEMAND       = 9   // Computed demand (read only)
EN_HEAD         = 10  // Computed head (read only)
EN_PRESSURE     = 11  // Computed pressure (read only)
EN_QUALITY      = 12  // Computed quality (read only)
```

**Link Properties (EN_LinkProperty)**:
```cpp
EN_STATUS       = 11  // Link status
EN_SETTING      = 12  // Pump/valve setting
EN_FLOW         = 8   // Computed flow (read only)
EN_VELOCITY     = 9   // Computed velocity (read only)
EN_HEADLOSS     = 10  // Computed head loss (read only)
EN_LINKQUAL     = 14  // Computed quality (read only)
EN_PUMP_EFFIC   = 17  // Computed efficiency (read only)
```

---

## 6. Implementation Details

### 6.1 Static State Variables

```cpp
static bool g_isInitialized = false;
static MappingConfig g_config;
static std::vector<InputMapping> g_inputs;
static std::vector<OutputMapping> g_outputs;
static Logger g_logger;
static bool g_logFileOpened = false;
static char g_errorBuffer[200];  // Must be static for XF_FAILURE_WITH_MSG
```

**Critical**: All state must be static to persist between function calls.

### 6.2 XF_INITIALIZE Implementation

```cpp
case XF_INITIALIZE:
{
    // 1. Load JSON configuration
    if (!MappingLoader::LoadConfig("EpanetBridge.json", g_config, error)) {
        SetErrorMessage(error, outargs, status);
        break;
    }
    
    // 2. Validate configuration
    if (!MappingLoader::ValidateConfig(g_config, error)) {
        SetErrorMessage(error, outargs, status);
        break;
    }
    
    // 3. Initialize logger
    LogLevel logLevel = ParseLogLevel(g_config.logging_level);
    g_logger.SetLevel(logLevel);
    if (!g_logFileOpened) {
        g_logger.SetLogFile("epanet_bridge_debug.log", false);
        g_logFileOpened = true;
    }
    
    // 4. Open EPANET model
    int errcode = EpanetWrapper::Open(g_config.inp_file.c_str(), "", "");
    if (errcode != 0) {
        SetErrorMessage("Failed to open EPANET model", outargs, status);
        break;
    }
    
    // 5. Resolve element indices
    if (!MappingLoader::ResolveIndices(g_config, error)) {
        EpanetWrapper::Close();
        SetErrorMessage(error, outargs, status);
        break;
    }
    
    // 6. Open hydraulics system
    errcode = EpanetWrapper::OpenHydraulics();
    if (errcode != 0) {
        EpanetWrapper::Close();
        SetErrorMessage("Failed to open hydraulics", outargs, status);
        break;
    }
    
    // 7. Initialize hydraulics
    errcode = EpanetWrapper::InitHydraulics(0);
    if (errcode != 0) {
        EpanetWrapper::CloseHydraulics();
        EpanetWrapper::Close();
        SetErrorMessage("Failed to initialize hydraulics", outargs, status);
        break;
    }
    
    // 8. Cache mappings
    g_inputs = g_config.inputs;
    g_outputs = g_config.outputs;
    g_isInitialized = true;
    
    *status = XF_SUCCESS;
}
```

### 6.3 XF_CALCULATE Implementation

```cpp
case XF_CALCULATE:
{
    // 1. Validate initialization
    if (!g_isInitialized) {
        SetErrorMessage("XF_CALCULATE called before XF_INITIALIZE", outargs, status);
        break;
    }
    
    // 2. Extract elapsed time
    double elapsedTime = inargs[0];
    
    // 3. Process all inputs (skip ElapsedTime at index 0)
    for (const InputMapping& input : g_inputs) {
        if (input.object_type == "SYSTEM") continue;
        
        double value = inargs[input.index];
        int errcode = 0;
        
        if (input.object_type == "NODE") {
            if (input.property == "DEMAND") {
                errcode = EpanetWrapper::SetNodeValue(input.epanet_index, 1, value);
            } else if (input.property == "EMITTER") {
                errcode = EpanetWrapper::SetNodeValue(input.epanet_index, 3, value);
            }
        } else if (input.object_type == "LINK") {
            if (input.property == "STATUS") {
                errcode = EpanetWrapper::SetLinkValue(input.epanet_index, 11, value);
            } else if (input.property == "SETTING") {
                errcode = EpanetWrapper::SetLinkValue(input.epanet_index, 12, value);
            }
        }
        
        if (errcode != 0) {
            SetErrorMessage("Failed to set input", outargs, status);
            return;
        }
    }
    
    // 4. Solve hydraulics to target time
    long currentTime = 0;
    long nextTime = 0;
    while (true) {
        int errcode = EpanetWrapper::SolveHydraulics(&currentTime);
        if (errcode > 100) {
            SetErrorMessage("Hydraulic solver failed", outargs, status);
            return;
        }
        
        if (currentTime >= (long)elapsedTime) break;
        
        errcode = EpanetWrapper::NextHydraulicStep(&nextTime);
        if (errcode != 0 || nextTime == 0) break;
    }
    
    // 5. Update tank volumes (critical for tank level accuracy)
    EpanetWrapper::NextHydraulicStep(&nextTime);
    
    // 6. Extract all outputs
    for (const OutputMapping& output : g_outputs) {
        double value = 0.0;
        int errcode = 0;
        
        if (output.object_type == "NODE") {
            if (output.property == "PRESSURE") {
                errcode = EpanetWrapper::GetNodeValue(output.epanet_index, 11, &value);
            } else if (output.property == "HEAD") {
                errcode = EpanetWrapper::GetNodeValue(output.epanet_index, 10, &value);
            } else if (output.property == "DEMAND") {
                errcode = EpanetWrapper::GetNodeValue(output.epanet_index, 9, &value);
            }
        } else if (output.object_type == "LINK") {
            if (output.property == "FLOW") {
                errcode = EpanetWrapper::GetLinkValue(output.epanet_index, 8, &value);
            } else if (output.property == "VELOCITY") {
                errcode = EpanetWrapper::GetLinkValue(output.epanet_index, 9, &value);
            } else if (output.property == "HEADLOSS") {
                errcode = EpanetWrapper::GetLinkValue(output.epanet_index, 10, &value);
            } else if (output.property == "EFFICIENCY") {
                errcode = EpanetWrapper::GetLinkValue(output.epanet_index, 17, &value);
            } else if (output.property == "POWER") {
                // Compute power from flow, head, and efficiency
                value = ComputePumpPower(output.epanet_index);
            }
        }
        
        if (errcode != 0) {
            SetErrorMessage("Failed to get output", outargs, status);
            return;
        }
        
        outargs[output.index] = value;
    }
    
    *status = XF_SUCCESS;
}
```

### 6.4 Pump Power Calculation

```cpp
double ComputePumpPower(int pumpIndex) {
    // Power (HP) = (Flow × Head × SG) / (3960 × Efficiency)
    // where Flow is GPM, Head is feet, Efficiency is fraction (0-1)
    
    double flow = 0.0;
    double headGain = 0.0;
    double efficiency = 0.0;
    double specificGravity = 1.0;
    
    // Get flow rate (EN_FLOW = 8)
    EpanetWrapper::GetLinkValue(pumpIndex, 8, &flow);
    
    // Get head gain (EN_HEADLOSS = 10, negative for pumps)
    EpanetWrapper::GetLinkValue(pumpIndex, 10, &headGain);
    headGain = -headGain;  // Pumps add head (negative headloss)
    
    // Get efficiency (EN_PUMP_EFFIC = 17)
    EpanetWrapper::GetLinkValue(pumpIndex, 17, &efficiency);
    
    // Compute power
    if (flow > 0.001 && efficiency > 0.001 && headGain > 0.0) {
        return (flow * headGain * specificGravity) / (3960.0 * efficiency);
    }
    return 0.0;
}
```

### 6.5 Error Handling

```cpp
static void SetErrorMessage(const std::string& message, double* outargs, int* status) {
    // Clear buffer
    memset(g_errorBuffer, 0, sizeof(g_errorBuffer));
    
    // Copy message (ensure NULL termination)
    strncpy_s(g_errorBuffer, sizeof(g_errorBuffer), message.c_str(), 
              sizeof(g_errorBuffer) - 1);
    g_errorBuffer[sizeof(g_errorBuffer) - 1] = '\0';
    
    // Store pointer in outargs[0]
    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
    *pAddr = (ULONG_PTR)g_errorBuffer;
    
    // Set error status
    *status = XF_FAILURE_WITH_MSG;
}
```

**Critical Requirements**:
- Error buffer must be static (persists after function return)
- Must be NULL-terminated
- Pointer stored in outargs[0] as ULONG_PTR

---

## 7. EPANET API Wrapper

### 7.1 Core Functions to Wrap

```cpp
// Project management
int ENopen(const char* inpFile, const char* rptFile, const char* outFile);
int ENclose();

// Hydraulics
int ENopenH();
int ENinitH(int initFlag);
int ENrunH(long* currentTime);
int ENnextH(long* nextTime);
int ENcloseH();

// Water quality (optional)
int ENopenQ();
int ENinitQ(int saveFlag);
int ENrunQ(long* currentTime);
int ENnextQ(long* nextTime);
int ENcloseQ();

// Element access
int ENgetnodeindex(const char* id, int* index);
int ENgetlinkindex(const char* id, int* index);
int ENgetnodevalue(int index, int property, double* value);
int ENsetnodevalue(int index, int property, double value);
int ENgetlinkvalue(int index, int property, double* value);
int ENsetlinkvalue(int index, int property, double value);

// Error handling
int ENgeterror(int errcode, char* errmsg, int maxLen);
```

### 7.2 Wrapper Class Structure

```cpp
class EpanetWrapper {
public:
    // Project management
    static int Open(const char* inpFile, const char* rptFile, const char* outFile);
    static int Close();
    static bool IsOpen();
    
    // Hydraulics
    static int OpenHydraulics();
    static int InitHydraulics(int initFlag);
    static int SolveHydraulics(long* currentTime);
    static int NextHydraulicStep(long* nextTime);
    static int CloseHydraulics();
    static bool IsHydraulicsOpen();
    
    // Water quality
    static int OpenQuality();
    static int InitQuality(int saveFlag);
    static int SolveQuality(long* currentTime);
    static int NextQualityStep(long* nextTime);
    static int CloseQuality();
    static bool IsQualityOpen();
    
    // Element access
    static int GetNodeIndex(const std::string& name, int* index);
    static int GetLinkIndex(const std::string& name, int* index);
    static int GetNodeValue(int index, int property, double* value);
    static int SetNodeValue(int index, int property, double value);
    static int GetLinkValue(int index, int property, double* value);
    static int SetLinkValue(int index, int property, double value);
    
    // Error handling
    static std::string GetErrorMessage(int errcode);
    
private:
    static bool s_isOpen;
    static bool s_hydraulicsOpen;
    static bool s_qualityOpen;
};
```

---

## 8. Configuration Validation

### 8.1 Schema Validation Rules

**Version**:
- Must be present
- Format: "X.Y" (e.g., "1.1")

**Logging Level**:
- Must be: OFF, NONE, ERROR, INFO, or DEBUG
- Case-sensitive
- Defaults to INFO if omitted

**Input/Output Counts**:
- Must match array lengths exactly
- Error if mismatch: "Argument count mismatch: JSON specifies X inputs but array contains Y entries"

**Object Types**:
- Inputs: SYSTEM, NODE, LINK, PATTERN
- Outputs: NODE, LINK only
- Case-sensitive

**Properties**:
- Must be valid for object type
- See section 5 for valid combinations
- Case-sensitive

### 8.2 Index Resolution

```cpp
bool MappingLoader::ResolveIndices(MappingConfig& config, std::string& error) {
    for (InputMapping& input : config.inputs) {
        // Skip SYSTEM and PATTERN (no EPANET indices)
        if (input.object_type == "SYSTEM" || input.object_type == "PATTERN") {
            input.epanet_index = 0;
            continue;
        }
        
        // Skip if already resolved
        if (input.epanet_index > 0) continue;
        
        // Resolve NODE indices
        if (input.object_type == "NODE") {
            int index = 0;
            int errorCode = EpanetWrapper::GetNodeIndex(input.name, &index);
            if (errorCode != 0) {
                error = "Element '" + input.name + "' not found in EPANET model";
                return false;
            }
            input.epanet_index = index;
        }
        
        // Resolve LINK indices
        else if (input.object_type == "LINK") {
            int index = 0;
            int errorCode = EpanetWrapper::GetLinkIndex(input.name, &index);
            if (errorCode != 0) {
                error = "Element '" + input.name + "' not found in EPANET model";
                return false;
            }
            input.epanet_index = index;
        }
    }
    
    // Repeat for outputs...
    return true;
}
```

---

## 9. Logging System

### 9.1 Log Levels

```cpp
enum class LogLevel {
    OFF = 0,    // No logging
    ERROR = 1,  // Errors only
    INFO = 2,   // Errors + informational messages
    DEBUG = 3   // Errors + info + debug details
};
```

### 9.2 Logger Class

```cpp
class Logger {
public:
    Logger();
    ~Logger();
    
    void SetLevel(LogLevel level);
    void SetLogFile(const std::string& filename, bool truncate);
    void Close();
    
    void Error(const std::string& message);
    void Info(const std::string& message);
    void Debug(const std::string& message);
    
private:
    LogLevel m_level;
    std::ofstream m_file;
    bool m_isOpen;
    
    void WriteLog(LogLevel level, const std::string& message);
    std::string GetTimestamp();
    std::string GetLevelString(LogLevel level);
};
```

### 9.3 Log Format

```
[YYYY-MM-DDTHH:MM:SS.mmm] [LEVEL] Message
```

Example:
```
[2026-02-09T18:07:58.853] [INFO] XF_INITIALIZE called - starting initialization
[2026-02-09T18:07:58.853] [DEBUG] Configuration version: 1.1
[2026-02-09T18:07:58.853] [DEBUG] EPANET model file: Pump_to_Tank.inp
[2026-02-09T18:07:58.854] [DEBUG] Pump POWER: flow=101.06 GPM, head=194.31 ft, eff=0.75, power=6.61 HP
```

### 9.4 What to Log

**ERROR Level**:
- Failed to load configuration
- Failed to open EPANET model
- Failed to resolve element indices
- Hydraulic solver failures
- Invalid property access

**INFO Level**:
- XF_INITIALIZE called
- Configuration loaded successfully
- EPANET model opened
- Initialization complete
- XF_CLEANUP called

**DEBUG Level**:
- XF_CALCULATE called with elapsed time
- Each input value being set
- Hydraulic solve progress
- Each output value extracted
- Computed values (e.g., pump power calculation details)

---

## 10. Build System

### 10.1 Visual Studio Project Settings

**Platform**: x64  
**Configuration**: Release (for distribution), Debug (for development)

**C/C++ Settings**:
- Language Standard: C++17 or later
- Runtime Library: Multi-threaded DLL (/MD)
- Additional Include Directories: `$(ProjectDir)..\include`
- Preprocessor Definitions: `_USRDLL`, `EPANETBRIDGE_EXPORTS`

**Linker Settings**:
- Additional Library Directories: `$(ProjectDir)..\lib`
- Additional Dependencies: `epanet2.lib`
- Output File: `gs_epanet.dll`
- Generate Import Library: Yes (`gs_epanet.lib`)

### 10.2 Build Script (build.ps1)

```powershell
param(
    [string]$Configuration = "Release",
    [string]$Platform = "x64"
)

$msbuildPath = "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"

if (-not (Test-Path $msbuildPath)) {
    Write-Error "MSBuild not found at $msbuildPath"
    exit 1
}

Write-Host "Building $Configuration|$Platform..."

& $msbuildPath `
    /p:Configuration=$Configuration `
    /p:Platform=$Platform `
    /m `
    EpanetGoldSimBridge.sln

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed"
    exit 1
}

# Copy output files
$outputDir = "bin\$Platform\$Configuration"
Copy-Item "$outputDir\gs_epanet.dll" "bin\$Platform\"
Copy-Item "$outputDir\gs_epanet.pdb" "bin\$Platform\"
Copy-Item "lib\epanet2.dll" "bin\$Platform\"

Write-Host "Build completed successfully!"
```

### 10.3 Dependencies

**Required Libraries**:
- EPANET 2.2+ (epanet2.dll, epanet2.lib)
- nlohmann/json (header-only, included in include/)
- Visual C++ Runtime (vcruntime140.dll, typically pre-installed)

**No External Dependencies**:
- No Boost
- No Qt
- No other third-party libraries

---

## 11. Testing Strategy

### 11.1 Unit Tests

**Test Files**:
- `test_logger_standalone.cpp` - Logger functionality
- `test_mapping_loader_standalone.cpp` - JSON parsing and validation
- `test_epanet_wrapper_standalone.cpp` - EPANET API wrapper

**Compile Command**:
```cmd
cl /EHsc /MD /W3 /O2 /I..\include test_logger_standalone.cpp Logger.obj
```

### 11.2 Integration Tests

**Test Files**:
- `test_xf_rep_arguments.cpp` - XF_REP_ARGUMENTS handler
- `test_tank_level_validation.cpp` - Tank level accuracy
- `test_pump_properties.cpp` - POWER and EFFICIENCY outputs

**Test Pattern**:
```cpp
int main() {
    int status = 0;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Test XF_REP_VERSION
    EpanetBridge(2, &status, inargs, outargs);
    assert(status == 0);
    assert(outargs[0] == 1.1);
    
    // Test XF_INITIALIZE
    EpanetBridge(0, &status, inargs, outargs);
    assert(status == 0);
    
    // Test XF_CALCULATE
    inargs[0] = 300.0;  // ElapsedTime
    inargs[1] = 1.0;    // Pump setting
    EpanetBridge(1, &status, inargs, outargs);
    assert(status == 0);
    assert(outargs[0] > 0);  // Tank head
    
    // Test XF_CLEANUP
    EpanetBridge(99, &status, inargs, outargs);
    assert(status == 0);
    
    return 0;
}
```

### 11.3 Test Runner Scripts

**PowerShell Test Runner**:
```powershell
# run_pump_properties_test.ps1

# Copy required files
Copy-Item ..\bin\x64\Release\gs_epanet.dll .
Copy-Item ..\lib\epanet2.dll .
Copy-Item test_data\config.json EpanetBridge.json

# Compile test
$vcvarsPath = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
$tempBat = "temp_compile.bat"
@"
@echo off
call "$vcvarsPath" >nul 2>&1
cl /EHsc /MD /W3 /O2 /I..\include test_pump_properties.cpp gs_epanet.lib
"@ | Out-File -FilePath $tempBat -Encoding ASCII

& cmd /c $tempBat
Remove-Item $tempBat

# Run test
& .\test_pump_properties.exe
exit $LASTEXITCODE
```

---

## 12. Common Pitfalls and Solutions

### 12.1 Memory Corruption Issues


**Problem**: Modifying inargs array  
**Symptom**: GoldSim crashes or produces incorrect results  
**Solution**: Never write to inargs - it's owned by GoldSim

**Problem**: Non-static error buffer  
**Symptom**: Garbage characters in error messages  
**Solution**: Error buffer must be static to persist after function return

**Problem**: Array bounds violations  
**Symptom**: Random crashes, memory corruption  
**Solution**: Always validate array indices against input_count/output_count

### 12.2 Tank Level Issues

**Problem**: Tank level doesn't change  
**Symptom**: Tank HEAD remains constant despite pump operation  
**Solution**: Call ENnextH after ENrunH to update tank volumes

**Problem**: Using TANKLEVEL property  
**Symptom**: Returns initial level parameter, not computed level  
**Solution**: Use HEAD property instead (HEAD = elevation + water level)

### 12.3 Timestep Synchronization

**Problem**: Results don't match standalone EPANET  
**Symptom**: Pressures/flows differ from EPANET GUI results  
**Solution**: Ensure GoldSim timestep matches EPANET hydraulic_timestep

**Problem**: Simulation advances too far  
**Symptom**: EPANET time exceeds GoldSim elapsed time  
**Solution**: Loop ENrunH/ENnextH until currentTime >= elapsedTime

### 12.4 Pump Power Issues

**Problem**: POWER always returns 0  
**Symptom**: Using EN_PUMP_POWER property code  
**Solution**: Compute power from flow, head, and efficiency (see section 6.4)

**Problem**: Efficiency shows 0.75 instead of 75  
**Symptom**: EPANET returns efficiency as fraction  
**Solution**: This is correct - multiply by 100 in GoldSim if needed

---

## 13. Distribution Package

### 13.1 Package Structure

```
EPANET_Bridge_v1.1/
├── bin/
│   └── x64/
│       ├── gs_epanet.dll
│       └── epanet2.dll
├── examples/
│   ├── Pump_to_Tank/
│   │   ├── Pump to Tank.gsm
│   │   ├── EpanetBridge.json
│   │   ├── Pump_to_Tank.inp
│   │   ├── gs_epanet.dll
│   │   └── epanet2.dll
│   ├── Example1_Simple/
│   ├── Example2_MultiJunction/
│   └── Example3_ValveControl/
├── scripts/
│   └── generate_mapping.py
├── README.md
├── LICENSE.txt
└── VERSION.txt
```

### 13.2 Version File Format

```
EPANET-GoldSim Bridge
Version: 1.1
Build Date: 2026-02-09
Platform: Windows x64

Contents:
- bin/x64/       : 64-bit DLL and dependencies
- examples/      : Example EPANET models and configurations
- scripts/       : Python mapping generator script
- README.md      : Main documentation
- LICENSE.txt    : MIT License
```

---

## 14. Performance Considerations

### 14.1 Typical Performance

**Initialization** (XF_INITIALIZE):
- < 1 second for networks with < 1000 nodes
- Dominated by EPANET model loading

**Calculation** (XF_CALCULATE):
- < 100ms per timestep for typical networks
- Scales with network size and convergence iterations

**Memory Overhead**:
- < 10MB beyond EPANET's own requirements
- Static state variables: ~1KB
- JSON configuration: ~10KB typical

### 14.2 Optimization Strategies

**Cache Mappings**:
- Store g_inputs and g_outputs vectors after XF_INITIALIZE
- Avoid repeated JSON parsing

**Minimize Logging**:
- Use INFO or ERROR level in production
- DEBUG level can slow down by 10-20%

**Efficient Property Access**:
- Use integer property codes directly
- Avoid string comparisons in hot paths

---

## 15. Security Considerations

### 15.1 Input Validation

**File Paths**:
- Validate inp_file exists before ENopen
- Sanitize paths to prevent directory traversal
- Limit to working directory if possible

**JSON Input**:
- Validate all required fields present
- Check array bounds before access
- Reject malformed JSON gracefully

**Array Access**:
- Always validate indices against counts
- Check for negative indices
- Prevent buffer overruns

### 15.2 Memory Safety

**Static Buffers**:
- Fixed-size error buffer (200 bytes)
- Always NULL-terminate strings
- Use strncpy_s or equivalent

**Resource Cleanup**:
- Always close EPANET in XF_CLEANUP
- Release file handles
- Clear sensitive data

---

## 16. Troubleshooting Guide

### 16.1 DLL Won't Load

**Check**:
- Architecture match (64-bit DLL with 64-bit GoldSim)
- epanet2.dll in same folder as gs_epanet.dll
- Visual C++ Runtime installed
- No missing dependencies (use Dependency Walker)

### 16.2 Configuration Errors

**Check**:
- EpanetBridge.json in same folder as .gsm file
- JSON syntax valid (use JSON validator)
- Element names match .inp file exactly (case-sensitive)
- input_count/output_count match array lengths

### 16.3 Incorrect Results

**Check**:
- GoldSim timestep matches hydraulic_timestep
- ElapsedTime connected to ETime
- Input values in valid ranges (e.g., SETTING 0.0-1.0)
- Tank levels using HEAD property, not TANKLEVEL

### 16.4 Debug Logging

**Enable DEBUG logging**:
```json
{
  "logging_level": "DEBUG"
}
```

**Check log file**: `epanet_bridge_debug.log` in working directory

**Key log entries**:
- Initialization success/failure
- Input values being set
- Hydraulic solve progress
- Output values extracted
- Error messages with context

---

## 17. Future Enhancements

### 17.1 Potential Features

**Additional Properties**:
- Pump energy consumption (kWh)
- Valve coefficients
- Pipe roughness updates
- Pattern manipulation

**Performance**:
- Parallel hydraulic solving
- Caching of repeated calculations
- Incremental updates

**Usability**:
- Python mapping generator (auto-create JSON from .inp)
- Configuration validation tool
- GoldSim model templates

### 17.2 API Extensions

**Batch Operations**:
- Set multiple inputs in single call
- Get multiple outputs efficiently

**Advanced Control**:
- Rule-based controls from GoldSim
- Time pattern updates
- Curve modifications

---

## 18. References

### 18.1 EPANET Documentation
- EPANET 2.2 User Manual: https://epanet22.readthedocs.io/
- EPANET Toolkit API: https://github.com/OpenWaterAnalytics/EPANET
- OWA-EPANET v2.3.3: https://github.com/OpenWaterAnalytics/EPANET/releases

### 18.2 GoldSim Documentation
- External Function API: GoldSim User Guide, Appendix B
- DLL Interface: GoldSim Help, "External (DLL) Element"

### 18.3 Related Projects
- GS-SWMM Bridge: Similar architecture for SWMM integration
- EPANET-MSX: Multi-species water quality extension

---

## Appendix A: Complete Example

### A.1 Minimal Working Implementation

See `examples/Pump_to_Tank/` for complete working example with:
- EPANET model (Pump_to_Tank.inp)
- JSON configuration (EpanetBridge.json)
- GoldSim model (Pump to Tank.gsm)
- Expected results and validation

### A.2 Test Configuration

```json
{
  "version": "1.1",
  "logging_level": "DEBUG",
  "inp_file": "Pump_to_Tank.inp",
  "hydraulic_timestep": 300,
  "input_count": 3,
  "output_count": 7,
  "inputs": [
    {"index": 0, "name": "ElapsedTime", "object_type": "SYSTEM", "property": "ELAPSEDTIME"},
    {"index": 1, "name": "Pump1", "object_type": "LINK", "property": "SETTING"},
    {"index": 2, "name": "Outlet", "object_type": "NODE", "property": "EMITTER"}
  ],
  "outputs": [
    {"index": 0, "name": "Tank1", "object_type": "NODE", "property": "HEAD"},
    {"index": 1, "name": "J2", "object_type": "NODE", "property": "PRESSURE"},
    {"index": 2, "name": "Pump1", "object_type": "LINK", "property": "FLOW"},
    {"index": 3, "name": "1", "object_type": "LINK", "property": "FLOW"},
    {"index": 4, "name": "2", "object_type": "LINK", "property": "FLOW"},
    {"index": 5, "name": "Pump1", "object_type": "LINK", "property": "POWER"},
    {"index": 6, "name": "Pump1", "object_type": "LINK", "property": "EFFICIENCY"}
  ]
}
```

---

**End of Document**

For questions or clarifications, refer to the source code in the repository or contact the development team.
