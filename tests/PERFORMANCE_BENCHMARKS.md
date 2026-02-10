# Performance Benchmarks

This document describes how to run performance benchmarks for the EPANET-GoldSim Bridge.

## Overview

The performance benchmarks validate that the bridge meets the performance requirements specified in Requirements 14.1-14.6:

- **Initialization Time**: < 1 second for networks with < 1000 nodes
- **Calculation Time**: < 100 milliseconds per timestep for networks with < 1000 nodes
- **Memory Overhead**: < 10 MB beyond EPANET's own requirements
- **Scalability**: Support networks up to 10,000 nodes without errors
- **File I/O**: No file I/O during XF_CALCULATE calls

## Prerequisites

1. **Visual Studio 2019 or later** with C++17 support
2. **EPANET 2.2 DLL** (epanet2.dll) in the lib/ directory
3. **Compiled gs_epanet.dll** bridge (in bin/x64/Debug or bin/x64/Release)
4. **Test data files** in tests/test_data/

## Running Benchmarks

### Method 1: Using the Batch Script (Recommended)

From a **Visual Studio Developer Command Prompt**:

```cmd
cd tests
compile_and_test_performance.bat
```

This will:
1. Compile the performance test executable
2. Copy required DLLs and test data
3. Run all 5 performance property tests
4. Display results with pass/fail status

### Method 2: Manual Compilation and Execution

From a **Visual Studio Developer Command Prompt**:

```cmd
cd tests

REM Compile
cl.exe /EHsc /std:c++17 /I..\include /I..\src ^
   property_tests\test_performance_properties.cpp ^
   /link /LIBPATH:..\lib epanet2.lib Psapi.lib ^
   /OUT:property_tests\test_performance_properties.exe

REM Copy dependencies
copy ..\lib\epanet2.dll property_tests\
copy ..\bin\x64\Debug\gs_epanet.dll property_tests\
xcopy /E /I test_data property_tests\test_data

REM Run
cd property_tests
test_performance_properties.exe
```

## Benchmark Tests

### Test 1: Initialization Performance (Property 15)

**Requirement**: XF_INITIALIZE completes in < 1 second for networks with < 1000 nodes

**Test Method**:
- Runs 10 initialization cycles
- Measures time for each XF_INITIALIZE call
- Reports average and maximum times
- Passes if maximum time < 1000ms

**Expected Result**: ~50-200ms for simple test network

### Test 2: Calculation Performance (Property 16)

**Requirement**: XF_CALCULATE completes in < 100 milliseconds for networks with < 1000 nodes

**Test Method**:
- Initializes bridge once
- Runs 20 XF_CALCULATE calls with advancing time
- Measures time for each calculation
- Reports average and maximum times
- Passes if maximum time < 100ms

**Expected Result**: ~10-50ms per calculation for simple test network

### Test 3: Memory Overhead (Property 17)

**Requirement**: Bridge adds < 10 MB memory overhead beyond EPANET

**Test Method**:
- Measures baseline process memory
- Opens EPANET directly and measures memory
- Initializes bridge and measures memory
- Calculates additional overhead beyond EPANET
- Passes if additional overhead < 10MB

**Expected Result**: ~2-5MB additional overhead

### Test 4: Scalability (Property 18)

**Requirement**: Support networks up to 10,000 nodes without errors

**Test Method**:
- Initializes bridge with test network
- Runs 10 calculation cycles
- Verifies all operations succeed without errors
- Cleans up successfully
- Passes if all operations succeed

**Expected Result**: All operations succeed (note: full test requires larger networks)

### Test 5: File I/O Minimization (Property 19)

**Requirement**: No file I/O during XF_CALCULATE calls

**Test Method**:
- Counts files in working directory before calculations
- Runs 20 XF_CALCULATE calls
- Counts files after calculations
- Passes if file count unchanged (no temporary files created)

**Expected Result**: No new files created during calculations

## Interpreting Results

### Sample Output

```
========================================
Performance Property-Based Tests
Feature: epanet-goldsim-bridge
Properties 15-19
========================================

=== Property 15: Initialization Performance ===
Testing with network: 5 nodes, 5 links
Average initialization time: 87.3 ms
Maximum initialization time: 142.1 ms
PASSED: Initialization time (142.1 ms) < 1000 ms

=== Property 16: Calculation Performance ===
Testing with 3 inputs, 4 outputs
Average calculation time: 23.5 ms
Maximum calculation time: 45.2 ms
PASSED: Calculation time (45.2 ms) < 100 ms

=== Property 17: Memory Overhead ===
Baseline memory: 12.5 MB
EPANET memory overhead: 3.2 MB
Bridge additional overhead: 2.1 MB
PASSED: Additional overhead (2.1 MB) < 10 MB

=== Property 18: Scalability ===
Testing with network: 5 nodes, 5 links
Initialization time: 89.4 ms
PASSED: Successfully completed 10 calculations without errors

=== Property 19: File I/O Minimization ===
Files before: 47, Files after: 47
PASSED: No temporary files created during calculations

========================================
Performance Tests Complete
========================================
```

### Pass/Fail Criteria

- **PASSED**: Test meets the performance requirement
- **FAILED**: Test exceeds the performance threshold
- **WARNING**: Test completed but with notes (e.g., file count changed due to logging)
- **SKIPPED**: Test cannot run (e.g., network too large)

## Troubleshooting

### Compilation Errors

**Error**: `'cl' is not recognized as an internal or external command`

**Solution**: Run from Visual Studio Developer Command Prompt, not regular Command Prompt

**Error**: `Cannot open include file: 'epanet2.h'`

**Solution**: Ensure EPANET headers are in `include/` directory

### Runtime Errors

**Error**: `Cannot open test network file`

**Solution**: Ensure test data files are copied to `property_tests/test_data/`

**Error**: `Could not initialize bridge`

**Solution**: 
- Check that gs_epanet.dll is in the same directory as the test executable
- Check that EpanetBridge.json exists in the working directory
- Check that the .inp file specified in the JSON exists

### Performance Issues

If tests fail due to performance:

1. **Check system load**: Close other applications
2. **Check build configuration**: Use Release build for accurate performance
3. **Check network size**: Verify test network is < 1000 nodes
4. **Check disk speed**: Slow disk I/O can affect initialization time

## Benchmarking Larger Networks

To test scalability with larger networks:

1. Create or obtain larger EPANET models (100, 1000, 10000 nodes)
2. Generate corresponding JSON configurations using `scripts/generate_mapping.py`
3. Update test code to use larger networks
4. Expect linear scaling: 10x nodes ≈ 10x time

## Continuous Integration

For CI/CD pipelines, run benchmarks as part of the build:

```yaml
- name: Run Performance Benchmarks
  run: |
    cd tests
    compile_and_test_performance.bat
  shell: cmd
```

Set thresholds in CI to fail builds if performance degrades.

## Performance Optimization Tips

If benchmarks fail:

1. **Profile the code**: Use Visual Studio Profiler to identify bottlenecks
2. **Optimize JSON parsing**: Cache parsed configuration
3. **Optimize EPANET calls**: Batch operations where possible
4. **Reduce logging**: Set logging level to ERROR or OFF for production
5. **Use Release build**: Debug builds are significantly slower

## References

- Requirements Document: `.kiro/specs/epanet-goldsim-bridge/requirements.md` (Section 14)
- Design Document: `.kiro/specs/epanet-goldsim-bridge/design.md` (Performance section)
- Property Tests: `tests/property_tests/test_performance_properties.cpp`
