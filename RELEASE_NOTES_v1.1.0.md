# Release Notes - Version 1.1.0

**Release Date:** February 9, 2026

## Overview

Version 1.1.0 adds support for monitoring pump power and efficiency properties in real-time, enabling energy analysis and pump performance monitoring within GoldSim simulations.

## New Features

### Pump Properties Support

Added two new output properties for pump monitoring:

1. **POWER** (EN_PUMP_POWER = 18)
   - Returns pump constant power rating in horsepower (HP)
   - Useful for energy analysis and pump sizing
   - Available for all pump types

2. **EFFICIENCY** (EN_PUMP_EFFIC = 17)
   - Returns current computed pump efficiency in percent (%)
   - Enables real-time monitoring of pump performance
   - Returns 0% when pump is off
   - Reflects actual operating efficiency based on pump curve and operating point

## Technical Changes

### Code Changes

- **EpanetBridge.cpp**: Added POWER and EFFICIENCY property handling in XF_CALCULATE
- **MappingLoader.cpp**: Updated property validation to accept POWER and EFFICIENCY
- **Version**: Incremented from 1.0 to 1.1

### Testing

- Created comprehensive test suite: `test_pump_properties.cpp`
- Test validates:
  - POWER output is non-negative
  - EFFICIENCY is in valid range [0, 100]
  - EFFICIENCY is positive when pump is on
  - EFFICIENCY is 0 when pump is off
- All existing tests continue to pass

### Documentation

- Updated README.md with new properties table
- Updated CHANGELOG.md with v1.1.0 release notes
- Created test configuration: `pump_properties_config.json`

## Usage Example

To monitor pump power and efficiency in your GoldSim model:

```json
{
  "outputs": [
    {
      "index": 0,
      "name": "Pump1",
      "object_type": "LINK",
      "property": "POWER"
    },
    {
      "index": 1,
      "name": "Pump1",
      "object_type": "LINK",
      "property": "EFFICIENCY"
    }
  ]
}
```

## Compatibility

- Fully backward compatible with v1.0.0 configurations
- No breaking changes
- Existing models will continue to work without modification

## Distribution

- Distribution package: `EPANET_Bridge_v1.1.zip`
- Package size: 0.77 MB
- Includes updated DLL, examples, and documentation

## Git Repository

- Branch: `main`
- Tag: `v1.1.0`
- Commit: Merge feature/add-pump-power-efficiency

## Testing Summary

All tests pass:
- ✅ Tank level validation test
- ✅ Pump properties test (14/14 tests passed)
- ✅ XF_REP_VERSION returns 1.1
- ✅ XF_REP_ARGUMENTS works correctly
- ✅ XF_INITIALIZE succeeds
- ✅ XF_CALCULATE with pump ON
- ✅ XF_CALCULATE with pump OFF
- ✅ XF_CLEANUP succeeds

## Known Issues

None identified in this release.

## Upgrade Instructions

1. Replace `gs_epanet.dll` with the new version
2. Optionally add POWER and/or EFFICIENCY outputs to your JSON configuration
3. No changes required to existing configurations

## Next Steps

Future enhancements may include:
- Additional pump properties (head, energy usage)
- Water quality properties for links
- Pattern manipulation capabilities
- Python mapping generator improvements

---

**For questions or issues, please refer to the project README.md or contact the development team.**
