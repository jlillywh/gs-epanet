# Python Mapping Generator

This directory contains the Python script for automatically generating JSON configuration files from EPANET .inp files.

## generate_mapping.py

Automatically generates `EpanetBridge.json` configuration from an EPANET model file.

### Usage

```powershell
python generate_mapping.py <inp_file> [options]
```

### Arguments

- `inp_file` - Path to EPANET .inp file (required)

### Options

- `--input <element> <property>` - Add an input mapping (can be used multiple times)
- `--output <element> <property>` - Add an output mapping (can be used multiple times)
- `--output-file <filename>` - Output JSON filename (default: EpanetBridge.json)
- `--quality` - Include water quality outputs for all nodes and links
- `--logging-level <level>` - Set logging level (OFF, ERROR, INFO, DEBUG)

### Examples

#### Generate with default outputs (all tanks, junctions, and links)

```powershell
python generate_mapping.py model.inp
```

#### Generate with custom inputs and outputs

```powershell
python generate_mapping.py model.inp `
    --input PUMP_1 SETTING `
    --input TANK_A DEMAND `
    --output TANK_A TANKLEVEL `
    --output TANK_B TANKLEVEL `
    --output PUMP_1 FLOW
```

#### Generate with water quality outputs

```powershell
python generate_mapping.py model.inp --quality
```

#### Generate with custom filename and logging

```powershell
python generate_mapping.py model.inp `
    --output-file MyConfig.json `
    --logging-level DEBUG
```

### Supported Properties

#### Input Properties

| Object Type | Property | Description |
|-------------|----------|-------------|
| NODE | DEMAND | Base demand at junction |
| LINK | STATUS | Link open (1) or closed (0) |
| LINK | SETTING | Pump speed or valve setting (0.0-1.0) |
| PATTERN | MULTIPLIER | Pattern multiplier |

#### Output Properties

| Object Type | Property | Description |
|-------------|----------|-------------|
| NODE | PRESSURE | Pressure at node |
| NODE | HEAD | Hydraulic head at node |
| NODE | DEMAND | Actual demand at node |
| NODE | TANKLEVEL | Water level in tank |
| NODE | QUALITY | Water quality concentration |
| LINK | FLOW | Flow rate through link |
| LINK | VELOCITY | Flow velocity in link |
| LINK | HEADLOSS | Head loss across link |
| LINK | STATUS | Link status (open/closed) |
| LINK | QUALITY | Average link quality |

### Output Format

The script generates a JSON file with the following structure:

```json
{
  "version": "1.0",
  "logging_level": "INFO",
  "inp_file": "model.inp",
  "inp_file_hash": "a3f5c8d9e2b1f4a6c7d8e9f0a1b2c3d4",
  "hydraulic_timestep": 300,
  "_comment": "IMPORTANT: Set GoldSim Basic Time Step to match hydraulic_timestep",
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
      "name": "PUMP_1",
      "object_type": "LINK",
      "property": "SETTING"
    }
  ],
  "outputs": [
    {
      "index": 0,
      "name": "TANK_A",
      "object_type": "NODE",
      "property": "TANKLEVEL"
    },
    {
      "index": 1,
      "name": "TANK_B",
      "object_type": "NODE",
      "property": "TANKLEVEL"
    },
    {
      "index": 2,
      "name": "PUMP_1",
      "object_type": "LINK",
      "property": "FLOW"
    }
  ]
}
```

### Notes

- ElapsedTime is always added as the first input automatically
- The script calculates an MD5 hash of the .inp file for validation
- The hydraulic timestep is extracted from the [TIMES] section
- A warning is displayed if the timestep is unusual (< 1 second or > 1 hour)
- Element names are case-sensitive and must match exactly as in the .inp file

### Requirements

- Python 3.11 or later
- No external dependencies (uses only standard library)

### Error Handling

The script will exit with an error if:
- The .inp file cannot be found or opened
- The .inp file is malformed or missing required sections
- An element name specified in --input or --output is not found in the model
- Invalid property names are specified

### Implementation Status

**Note**: This script will be implemented in Task 13 of the implementation plan. For now, this is a placeholder README describing the planned functionality.
