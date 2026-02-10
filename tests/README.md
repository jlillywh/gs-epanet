# EPANET-GoldSim Bridge Tests

This directory contains unit tests, property-based tests, and a GoldSim simulator for the EPANET-GoldSim Bridge.

## GoldSim Simulator

**NEW**: A standalone application that simulates how GoldSim calls the EPANET Bridge DLL!

The GoldSim Simulator (`goldsim_simulator.cpp`) is a Windows executable that mimics GoldSim's interaction with the bridge DLL. It's perfect for:
- Testing the bridge without requiring GoldSim
- Debugging and development
- Automated testing and CI/CD
- Understanding the GoldSim External Function API

**Quick Start**:
```cmd
# From Visual Studio Developer Command Prompt
cd tests
compile_simulator_simple.bat
```

This compiles and runs the simulator with 10 timesteps using Example2 (Multi-Junction Network).

**Documentation**:
- **Quick Reference**: `SIMULATOR_QUICK_REFERENCE.md` - One-page guide
- **Complete Guide**: `GOLDSIM_SIMULATOR.md` - Full documentation with troubleshooting

**Features**:
- Executes complete GoldSim API lifecycle (VERSION → ARGUMENTS → INITIALIZE → CALCULATE → CLEANUP)
- Simulates realistic time-varying demands (residential, commercial, industrial patterns)
- Displays results for each timestep (pressures, flows)
- Generates detailed logs for debugging
- Configurable number of timesteps (default: 10)

See `GOLDSIM_SIMULATOR.md` for complete documentation.

**Test Status**: ✅ **PASSED** (February 7, 2026)
- Successfully executed 10 timesteps with Example2 Multi-Junction Network
- All 5 GoldSim API methods working correctly
- 100% convergence rate (1-4 trials per timestep)
- Execution time: < 1 second for 10 timesteps
- See `TEST_SUMMARY.md` and `GOLDSIM_SIMULATOR_TEST_RESULTS.md` for detailed results

## Test Framework

- **Unit Tests**: Google Test (gtest)
- **Property-Based Tests**: RapidCheck

## Setup

### Installing RapidCheck

RapidCheck is a QuickCheck clone for C++ used for property-based testing.

#### Option 1: Build from Source
```powershell
# Clone the repository
git clone https://github.com/emil-e/rapidcheck.git
cd rapidcheck

# Build using CMake
mkdir build
cd build
cmake .. -G "Visual Studio 17 2026"
cmake --build . --config Release

# Install (or copy headers and libs to your project)
cmake --install . --prefix C:\path\to\install
```

#### Option 2: Use vcpkg
```powershell
vcpkg install rapidcheck:x64-windows
vcpkg install rapidcheck:x86-windows
```

#### Option 3: Use Conan
```powershell
conan install rapidcheck/cci.20220514@
```

### Installing Google Test

Google Test is included as a submodule or can be installed via vcpkg:

```powershell
vcpkg install gtest:x64-windows
vcpkg install gtest:x86-windows
```

## Running Tests

### Prerequisites

All tests require the Visual Studio 2026 C++ compiler (cl.exe) to be in your PATH. You can run tests from:

1. **Developer Command Prompt for VS 2026** (recommended)
2. **x64 Native Tools Command Prompt for VS 2026**
3. Regular command prompt after running:
   ```batch
   "C:\Program Files\Microsoft Visual Studio\2026\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
   ```

### Run All Tests
```powershell
.\run_all_tests.bat
```

### Run Specific Test Suite
```powershell
.\unit_tests.exe --gtest_filter=EpanetBridgeTest.*
```

### Run with Memory Leak Detection
```powershell
drmemory -batch -- .\unit_tests.exe
```

## Test Structure

```
tests/
├── unit_tests/           # Unit tests for specific functionality
│   ├── LoggerTest.cpp
│   └── ...
├── property_tests/       # Property-based tests
│   ├── test_performance_properties.cpp  # Performance tests (Properties 15-19)
│   └── README.md
├── integration_tests/    # End-to-end integration tests
│   └── ...
├── test_data/           # Test EPANET models and configurations
│   ├── simple_network.inp
│   ├── simple_test.inp
│   ├── valid_config.json
│   └── ...
├── compile_and_test_*.bat  # Individual test compilation scripts
└── README.md
```

## Standalone Tests

The following standalone tests can be compiled and run individually:

- `test_logger_standalone.cpp` - Tests Logger component
- `test_mapping_loader_standalone.cpp` - Tests MappingLoader component
- `test_epanet_wrapper_standalone.cpp` - Tests EpanetWrapper component
- `test_xf_rep_arguments.cpp` - Tests XF_REP_ARGUMENTS handler
- `test_value_accessors.cpp` - Tests EPANET value accessor methods

Each has a corresponding `compile_and_test_*.bat` script.

## Property-Based Tests

Property-based tests validate universal correctness properties across many inputs:

### Performance Tests (Properties 15-19)
Located in `property_tests/test_performance_properties.cpp`:

- **Property 15**: Initialization Performance (< 1 second for < 1000 nodes)
- **Property 16**: Calculation Performance (< 100ms for < 1000 nodes)
- **Property 17**: Memory Overhead (< 10MB beyond EPANET)
- **Property 18**: Scalability (up to 10,000 nodes without errors)
- **Property 19**: File I/O Minimization (no file I/O during calculations)

Run with: `compile_and_test_performance.bat`

See `property_tests/README.md` for detailed information.

## Memory Leak Detection

Memory leak detection and cleanup verification tests are located in:
- `test_memory_leak_verification.cpp` - Comprehensive memory management tests
- `MEMORY_LEAK_DETECTION.md` - Detailed documentation and best practices

Run with: `compile_and_test_memory_leak.bat`

These tests validate:
- Multiple initialize-cleanup cycles (Requirements 9.1, 9.2)
- File handle cleanup (Requirement 9.5)
- EPANET resource management
- Memory stability under load
- Exception safety (Requirement 9.7)

For advanced memory leak detection, use external tools like Dr. Memory or Visual Studio Diagnostic Tools. See `MEMORY_LEAK_DETECTION.md` for details.

## Test Coverage Goals

- **Line Coverage**: > 80%
- **Branch Coverage**: > 75%
- **Property Test Iterations**: 100 per property
- **Integration Test Success Rate**: 100%
- **Memory Leak Detection**: Zero leaks
