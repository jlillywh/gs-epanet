# Performance Property-Based Tests

This directory contains property-based tests for performance requirements (Properties 15-19).

## Test Files

- `test_performance_properties.cpp` - Performance property tests for initialization, calculation, memory, scalability, and file I/O

## Properties Tested

### Property 15: Initialization Performance
**Validates: Requirements 14.1**

For any EPANET network with fewer than 1000 nodes, XF_INITIALIZE should complete in less than 1 second.

### Property 16: Calculation Performance
**Validates: Requirements 14.2**

For any EPANET network with fewer than 1000 nodes, XF_CALCULATE should complete in less than 100 milliseconds.

### Property 17: Memory Overhead
**Validates: Requirements 14.3**

The Bridge_DLL should add less than 10 MB of memory overhead beyond EPANET's own requirements.

### Property 18: Scalability
**Validates: Requirements 14.5**

For networks with up to 10,000 nodes and 10,000 links, the Bridge_DLL should successfully initialize, calculate, and cleanup without errors or performance degradation beyond linear scaling.

### Property 19: File I/O Minimization
**Validates: Requirements 14.6**

For any sequence of XF_CALCULATE calls within a single realization, no file I/O operations should occur (all data should be processed in memory).

## Compilation

To compile the performance tests, run from the `tests` directory in a Visual Studio Developer Command Prompt:

```cmd
compile_and_test_performance.bat
```

Or manually compile:

```cmd
cl.exe /EHsc /std:c++17 /I..\include /I..\src ^
   property_tests\test_performance_properties.cpp ^
   /link /LIBPATH:..\lib epanet2.lib Psapi.lib ^
   /OUT:property_tests\test_performance_properties.exe
```

## Running Tests

After compilation, the tests will run automatically. You can also run them manually:

```cmd
cd property_tests
test_performance_properties.exe
```

## Requirements

- Visual Studio 2019 or later (for C++17 support)
- EPANET 2.2 DLL (epanet2.dll)
- Compiled gs_epanet.dll bridge
- Test data files in `tests/test_data/`

## Test Configuration

The tests use the following configuration:
- **Initialization test**: 10 iterations, threshold 1000ms
- **Calculation test**: 20 iterations, threshold 100ms
- **Memory test**: Compares bridge overhead to direct EPANET usage
- **Scalability test**: 10 calculation iterations
- **File I/O test**: 20 calculation iterations, monitors file creation

## Notes

- These are optional performance tests (marked with `*` in tasks.md)
- Tests use the simple_network.inp test model (< 1000 nodes)
- For full scalability testing (Property 18), larger test networks (up to 10,000 nodes) would be needed
- Memory measurements use Windows Process Memory Counters
- File I/O monitoring checks for temporary file creation during calculations

## Expected Results

With the simple test network:
- Initialization: < 100ms (well under 1 second threshold)
- Calculation: < 50ms (well under 100ms threshold)
- Memory overhead: < 5MB (well under 10MB threshold)
- All calculations should succeed without errors
- No temporary files should be created during calculations
