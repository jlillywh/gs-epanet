# Changelog

All notable changes to the EPANET-GoldSim Bridge project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2026-02-09

### Added
- Support for pump POWER output property (computed from flow, head, and efficiency)
  - Returns actual pump power consumption in horsepower
  - Formula: Power (HP) = (Flow × Head × SG) / (3960 × Efficiency)
  - Useful for energy analysis and operating cost calculations
- Support for pump EFFICIENCY output property (EN_PUMP_EFFIC = 17)
  - Returns current computed pump efficiency as a fraction (0.75 = 75%)
  - Enables monitoring of pump performance at each timestep

### Changed
- Updated DLL version from 1.0 to 1.1
- Enhanced MappingLoader validation to accept POWER and EFFICIENCY properties

## [1.0.0] - 2026-02-09

### Added
- Initial stable release
- GoldSim External Function API implementation (XF_INITIALIZE, XF_CALCULATE, XF_REP_VERSION, XF_REP_ARGUMENTS, XF_CLEANUP)
- JSON-based configuration system for flexible input/output mapping
- Support for controlling EPANET elements from GoldSim:
  - Pump speed/status (LINK SETTING)
  - Valve settings (LINK SETTING, STATUS)
  - Node emitter coefficients (NODE EMITTER)
  - Junction demands (NODE DEMAND)
- Support for monitoring EPANET results in GoldSim:
  - Node pressures and hydraulic head (NODE PRESSURE, HEAD)
  - Link flows and velocities (LINK FLOW, VELOCITY)
  - Tank levels (NODE HEAD - use HEAD instead of TANKLEVEL)
- Comprehensive error handling with descriptive messages
- Configurable logging (OFF, ERROR, INFO, DEBUG levels)
- EPANET 2.3.3 integration
- Pump_to_Tank working example with deadband controller
- Automated tank level validation test
- Build system for Visual Studio 2022/2026
- Complete user documentation in README.md

### Fixed
- Tank level updates now work correctly by calling ENnextH after solving hydraulics
- Use HEAD property instead of TANKLEVEL for accurate tank level monitoring

### Known Issues
- TANKLEVEL property returns initial level parameter, not computed level (use HEAD instead)
- Python mapping generator script not yet implemented (manual JSON configuration required)

### Technical Details
- DLL Version: 1.0
- EPANET Version: 2.3.3
- Target Platform: Windows 10/11 (64-bit)
- Build Tools: Visual Studio 2022/2026, MSBuild 17.0+
