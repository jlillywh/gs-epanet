# Memory Leak Detection and Cleanup Verification

## Overview

This document describes the memory leak detection and cleanup verification strategy for the EPANET-GoldSim Bridge. The tests validate that the bridge properly manages memory and resources according to Requirements 9.1, 9.2, 9.5, and 9.7.

## Test Strategy

### Prerequisites

Before running the memory leak tests, ensure you have:

1. **Visual Studio 2026** installed with C++ development tools
2. **Developer Command Prompt** - Run tests from "Developer Command Prompt for VS 2026" or "x64 Native Tools Command Prompt for VS 2026"
3. **EPANET 2.2 DLL** - Located in `lib/epanet2.dll`
4. **Test data files** - Located in `tests/test_data/`

To set up the environment in a regular command prompt:

```batch
REM Locate and run vcvarsall.bat to set up the build environment
"C:\Program Files\Microsoft Visual Studio\2026\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
```

### Automated Memory Tests

The `test_memory_leak_verification.cpp` test suite includes:

1. **Multiple Initialize-Cleanup Cycles**: Runs 10 complete initialization and cleanup cycles to detect memory leaks
2. **File Handle Cleanup**: Verifies that log files and other file handles are properly closed
3. **EPANET Resource Cleanup**: Tests that EPANET resources can be properly released and re-initialized
4. **Cleanup Without Initialization**: Ensures graceful handling of cleanup calls without prior initialization
5. **Memory Stability Under Load**: Runs 100 calculations to verify no memory leaks during normal operation

### Running the Tests

```batch
cd tests
compile_and_test_memory_leak.bat
```

### Expected Results

- **Memory Growth**: Less than 1 MB growth over 10 initialize-cleanup cycles
- **Calculation Memory**: Less than 500 KB growth during 100 calculations
- **File Handles**: All file handles closed after cleanup
- **Re-initialization**: Successful re-initialization after cleanup

## Manual Memory Leak Detection

For more comprehensive memory leak detection, use external tools:

### Option 1: Visual Studio Diagnostic Tools

1. Open the project in Visual Studio 2026
2. Go to Debug → Performance Profiler
3. Select "Memory Usage"
4. Run the test executable
5. Take snapshots before and after test cycles
6. Compare snapshots to identify leaks

### Option 2: Dr. Memory (Windows)

Dr. Memory is a memory monitoring tool for Windows:

```batch
# Download Dr. Memory from https://drmemory.org/
# Install and add to PATH

# Run tests with Dr. Memory
drmemory.exe -- test_memory_leak_verification.exe

# Check the results in the generated log file
```

### Option 3: Application Verifier (Windows)

Application Verifier is a Microsoft tool for detecting memory issues:

```batch
# Install Application Verifier from Windows SDK
# Configure it to monitor the test executable

appverif.exe /verify test_memory_leak_verification.exe
test_memory_leak_verification.exe

# Check for errors in Event Viewer
```

## Memory Management Best Practices

### RAII Patterns

The bridge uses RAII (Resource Acquisition Is Initialization) patterns to ensure proper cleanup:

```cpp
// Logger uses RAII for file handles
class Logger {
    std::ofstream m_file;  // Automatically closed in destructor
public:
    ~Logger() {
        if (m_file.is_open()) {
            m_file.close();
        }
    }
};
```

### Smart Pointers

Use smart pointers for dynamic memory:

```cpp
// Use std::unique_ptr for owned resources
std::unique_ptr<MappingConfig> config = std::make_unique<MappingConfig>();

// Use std::shared_ptr for shared resources
std::shared_ptr<Logger> logger = std::make_shared<Logger>();
```

### Cleanup Checklist

The XF_CLEANUP handler must:

1. ✓ Call `EpanetWrapper::CloseHydraulics()`
2. ✓ Call `EpanetWrapper::Close()`
3. ✓ Close logger file with `Logger::Close()`
4. ✓ Clear all static state variables
5. ✓ Set `g_isInitialized = false`
6. ✓ Release any dynamically allocated memory

## Common Memory Issues

### Issue 1: File Handle Leaks

**Symptom**: Log file cannot be deleted or opened after cleanup

**Solution**: Ensure `Logger::Close()` is called in XF_CLEANUP

```cpp
void XF_CLEANUP_Handler() {
    g_logger.Close();  // Must close file handle
    // ... other cleanup
}
```

### Issue 2: EPANET Resource Leaks

**Symptom**: Cannot re-initialize after cleanup, or memory grows with each cycle

**Solution**: Call both `ENcloseH()` and `ENclose()` in proper order

```cpp
void XF_CLEANUP_Handler() {
    EpanetWrapper::CloseHydraulics();  // First close hydraulics
    EpanetWrapper::Close();             // Then close EPANET
}
```

### Issue 3: Static Variable Leaks

**Symptom**: Memory grows with each initialize-cleanup cycle

**Solution**: Clear all static containers in XF_CLEANUP

```cpp
void XF_CLEANUP_Handler() {
    g_inputs.clear();
    g_outputs.clear();
    g_config = MappingConfig();  // Reset to default
    g_isInitialized = false;
}
```

### Issue 4: Exception Safety

**Symptom**: Memory leaks when exceptions occur

**Solution**: Use RAII and smart pointers to ensure cleanup on exceptions

```cpp
void XF_INITIALIZE_Handler() {
    try {
        // Use RAII objects that clean up automatically
        std::unique_ptr<MappingConfig> config = LoadConfig();
        // ... initialization
    } catch (...) {
        // RAII objects automatically cleaned up
        throw;
    }
}
```

## Validation Criteria

### Pass Criteria

- ✓ All 5 automated tests pass
- ✓ Memory growth < 1 MB over 10 cycles
- ✓ Memory growth < 500 KB during 100 calculations
- ✓ All file handles closed after cleanup
- ✓ Successful re-initialization after cleanup
- ✓ No crashes or exceptions during tests

### Fail Criteria

- ✗ Memory growth > 1 MB over 10 cycles
- ✗ File handles remain open after cleanup
- ✗ Cannot re-initialize after cleanup
- ✗ Crashes or exceptions during tests
- ✗ External tools (Dr. Memory, etc.) report leaks

## Integration with CI/CD

Add memory leak tests to the automated test suite:

```batch
REM In run_all_tests.bat
call compile_and_test_memory_leak.bat
if %ERRORLEVEL% NEQ 0 (
    echo Memory leak tests failed!
    exit /b 1
)
```

## References

- **Requirement 9.1**: Memory allocation during XF_INITIALIZE
- **Requirement 9.2**: Memory release during XF_CLEANUP
- **Requirement 9.5**: File handle cleanup
- **Requirement 9.7**: Memory allocation failure handling

## Tools and Resources

- [Dr. Memory](https://drmemory.org/) - Memory monitoring tool for Windows
- [Visual Studio Diagnostic Tools](https://docs.microsoft.com/en-us/visualstudio/profiling/) - Built-in profiling
- [Application Verifier](https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/application-verifier) - Microsoft memory checker
- [Windows Performance Analyzer](https://docs.microsoft.com/en-us/windows-hardware/test/wpt/) - Advanced profiling

## Maintenance

This test suite should be run:

- Before each release
- After any changes to memory management code
- After changes to XF_INITIALIZE or XF_CLEANUP handlers
- When investigating reported memory issues
- As part of the CI/CD pipeline

## Contact

For questions about memory leak detection or test failures, refer to the project README.md or contact the development team.
