#!/usr/bin/env python3
"""
EPANET-GoldSim Bridge Mapping Generator

This script automatically generates a JSON configuration file for the EPANET-GoldSim
Bridge by parsing an EPANET .inp file. The JSON file defines the mapping between
GoldSim inputs/outputs and EPANET network elements.

Usage:
    python generate_mapping.py model.inp [options]

Options:
    --input NODE_ID:PROPERTY    Add an input mapping (can be used multiple times)
    --output NODE_ID:PROPERTY   Add an output mapping (can be used multiple times)
    --output-file FILENAME      Output JSON filename (default: EpanetBridge.json)
    --quality                   Include water quality outputs for all nodes/links
    --logging-level LEVEL       Set logging level (OFF, ERROR, INFO, DEBUG)

Examples:
    # Generate with default outputs (all tanks, junctions, links)
    python generate_mapping.py network.inp

    # Generate with custom inputs and outputs
    python generate_mapping.py network.inp --input TANK_1:DEMAND --output JUNCTION_5:PRESSURE

    # Generate with water quality outputs
    python generate_mapping.py network.inp --quality

Requirements: 6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7, 6.8
"""

import argparse
import hashlib
import json
import os
import re
import sys
from typing import Dict, List, Tuple, Optional


class EpanetInpParser:
    """Parser for EPANET .inp files"""
    
    def __init__(self, inp_file: str):
        self.inp_file = inp_file
        self.junctions: List[str] = []
        self.reservoirs: List[str] = []
        self.tanks: List[str] = []
        self.pipes: List[str] = []
        self.pumps: List[str] = []
        self.valves: List[str] = []
        self.patterns: List[str] = []
        self.hydraulic_timestep: int = 3600  # Default 1 hour
        self.quality_enabled: bool = False  # Whether water quality simulation is enabled
        
    def parse(self) -> bool:
        """
        Parse the EPANET .inp file and extract all element IDs
        
        Returns:
            True if parsing succeeded, False otherwise
        """
        try:
            with open(self.inp_file, 'r') as f:
                content = f.read()
        except FileNotFoundError:
            print(f"ERROR: Input file '{self.inp_file}' not found", file=sys.stderr)
            return False
        except Exception as e:
            print(f"ERROR: Failed to read input file: {e}", file=sys.stderr)
            return False
        
        # Parse each section
        self._parse_section(content, '[JUNCTIONS]', self.junctions)
        self._parse_section(content, '[RESERVOIRS]', self.reservoirs)
        self._parse_section(content, '[TANKS]', self.tanks)
        self._parse_section(content, '[PIPES]', self.pipes)
        self._parse_section(content, '[PUMPS]', self.pumps)
        self._parse_section(content, '[VALVES]', self.valves)
        self._parse_section(content, '[PATTERNS]', self.patterns)
        self._parse_times_section(content)
        self._parse_quality_section(content)
        
        return True
    
    def _parse_section(self, content: str, section_name: str, output_list: List[str]):
        """Parse a section and extract element IDs (first column)"""
        # Find section start
        section_match = re.search(rf'{re.escape(section_name)}', content, re.IGNORECASE)
        if not section_match:
            return
        
        # Find section end (next section or end of file)
        section_start = section_match.end()
        next_section = re.search(r'\n\[', content[section_start:])
        if next_section:
            section_end = section_start + next_section.start()
        else:
            section_end = len(content)
        
        section_content = content[section_start:section_end]
        
        # Parse lines
        for line in section_content.split('\n'):
            # Remove comments
            line = line.split(';')[0].strip()
            if not line:
                continue
            
            # Extract first column (element ID)
            parts = line.split()
            if parts:
                element_id = parts[0]
                output_list.append(element_id)
    
    def _parse_times_section(self, content: str):
        """Parse [TIMES] section to extract HYDRAULIC TIMESTEP"""
        section_match = re.search(r'\[TIMES\]', content, re.IGNORECASE)
        if not section_match:
            return
        
        section_start = section_match.end()
        next_section = re.search(r'\n\[', content[section_start:])
        if next_section:
            section_end = section_start + next_section.start()
        else:
            section_end = len(content)
        
        section_content = content[section_start:section_end]
        
        # Look for "Hydraulic Timestep" line
        for line in section_content.split('\n'):
            # Remove comments
            line = line.split(';')[0].strip()
            if not line:
                continue
            
            # Check if this is the hydraulic timestep line
            if re.match(r'Hydraulic\s+Timestep', line, re.IGNORECASE):
                parts = line.split()
                if len(parts) >= 3:
                    # Parse time value (format: "Hydraulic Timestep 0:05" or "Hydraulic Timestep 0:05:00")
                    time_str = parts[2]
                    self.hydraulic_timestep = self._parse_time_string(time_str)
                    break
    
    def _parse_time_string(self, time_str: str) -> int:
        """
        Parse EPANET time string to seconds
        
        Formats supported:
        - "0:05" (5 minutes)
        - "1:00" (1 hour)
        - "0:05:00" (5 minutes)
        - "1:00:00" (1 hour)
        
        Returns:
            Time in seconds
        """
        parts = time_str.split(':')
        if len(parts) == 2:
            # Format: H:MM
            hours = int(parts[0])
            minutes = int(parts[1])
            return hours * 3600 + minutes * 60
        elif len(parts) == 3:
            # Format: H:MM:SS
            hours = int(parts[0])
            minutes = int(parts[1])
            seconds = int(parts[2])
            return hours * 3600 + minutes * 60 + seconds
        else:
            # Default to 1 hour if parsing fails
            return 3600
    
    def _parse_quality_section(self, content: str):
        """
        Parse [OPTIONS] section to detect if water quality simulation is enabled
        
        Water quality is enabled if the Quality option is set to anything other than NONE.
        Possible values: NONE, CHEMICAL, AGE, TRACE
        """
        section_match = re.search(r'\[OPTIONS\]', content, re.IGNORECASE)
        if not section_match:
            return
        
        section_start = section_match.end()
        next_section = re.search(r'\n\[', content[section_start:])
        if next_section:
            section_end = section_start + next_section.start()
        else:
            section_end = len(content)
        
        section_content = content[section_start:section_end]
        
        # Look for "Quality" line
        for line in section_content.split('\n'):
            # Remove comments
            line = line.split(';')[0].strip()
            if not line:
                continue
            
            # Check if this is the quality option line
            if re.match(r'Quality', line, re.IGNORECASE):
                parts = line.split()
                if len(parts) >= 2:
                    quality_type = parts[1].upper()
                    # Quality is enabled if it's not NONE
                    self.quality_enabled = (quality_type != 'NONE')
                    break
    
    def get_all_nodes(self) -> List[str]:
        """Get all node IDs (junctions + reservoirs + tanks)"""
        return self.junctions + self.reservoirs + self.tanks
    
    def get_all_links(self) -> List[str]:
        """Get all link IDs (pipes + pumps + valves)"""
        return self.pipes + self.pumps + self.valves


class MappingGenerator:
    """Generator for EPANET-GoldSim Bridge JSON configuration"""
    
    def __init__(self, parser: EpanetInpParser, inp_file: str):
        self.parser = parser
        self.inp_file = inp_file
        self.inputs: List[Dict] = []
        self.outputs: List[Dict] = []
        
    def add_input(self, element_spec: str) -> bool:
        """
        Add an input mapping from specification string
        
        Format: "ELEMENT_ID:PROPERTY"
        Example: "TANK_1:DEMAND", "PUMP_1:SETTING"
        
        Returns:
            True if input was added successfully, False otherwise
        """
        parts = element_spec.split(':')
        if len(parts) != 2:
            print(f"ERROR: Invalid input specification '{element_spec}'. Expected format: ELEMENT_ID:PROPERTY", 
                  file=sys.stderr)
            return False
        
        element_id, property_name = parts
        
        # Determine object type
        object_type = self._get_object_type(element_id)
        if not object_type:
            print(f"ERROR: Element '{element_id}' not found in EPANET model", file=sys.stderr)
            return False
        
        # Validate property for object type
        if not self._is_valid_input_property(object_type, property_name):
            print(f"ERROR: Property '{property_name}' is not valid for {object_type} inputs", file=sys.stderr)
            return False
        
        # Add input (index will be set later)
        self.inputs.append({
            "index": len(self.inputs) + 1,  # +1 for ElapsedTime at index 0
            "name": element_id,
            "object_type": object_type,
            "property": property_name
        })
        
        return True
    
    def add_output(self, element_spec: str) -> bool:
        """
        Add an output mapping from specification string
        
        Format: "ELEMENT_ID:PROPERTY"
        Example: "JUNCTION_1:PRESSURE", "PIPE_1:FLOW"
        
        Returns:
            True if output was added successfully, False otherwise
        """
        parts = element_spec.split(':')
        if len(parts) != 2:
            print(f"ERROR: Invalid output specification '{element_spec}'. Expected format: ELEMENT_ID:PROPERTY", 
                  file=sys.stderr)
            return False
        
        element_id, property_name = parts
        
        # Determine object type
        object_type = self._get_object_type(element_id)
        if not object_type:
            print(f"ERROR: Element '{element_id}' not found in EPANET model", file=sys.stderr)
            return False
        
        # Validate property for object type
        if not self._is_valid_output_property(object_type, property_name):
            print(f"ERROR: Property '{property_name}' is not valid for {object_type} outputs", file=sys.stderr)
            return False
        
        # Add output
        self.outputs.append({
            "index": len(self.outputs),
            "name": element_id,
            "object_type": object_type,
            "property": property_name
        })
        
        return True
    
    def generate_default_outputs(self, include_quality: bool = False):
        """
        Generate default outputs: all tanks (TANKLEVEL), all junctions (PRESSURE), all links (FLOW)
        
        Args:
            include_quality: If True, also add QUALITY outputs for all nodes and links
        """
        # Add tank levels
        for tank in self.parser.tanks:
            self.outputs.append({
                "index": len(self.outputs),
                "name": tank,
                "object_type": "NODE",
                "property": "TANKLEVEL"
            })
        
        # Add junction pressures
        for junction in self.parser.junctions:
            self.outputs.append({
                "index": len(self.outputs),
                "name": junction,
                "object_type": "NODE",
                "property": "PRESSURE"
            })
        
        # Add link flows
        for link in self.parser.get_all_links():
            self.outputs.append({
                "index": len(self.outputs),
                "name": link,
                "object_type": "LINK",
                "property": "FLOW"
            })
        
        # Add quality outputs if requested
        if include_quality:
            for node in self.parser.get_all_nodes():
                self.outputs.append({
                    "index": len(self.outputs),
                    "name": node,
                    "object_type": "NODE",
                    "property": "QUALITY"
                })
            
            for link in self.parser.get_all_links():
                self.outputs.append({
                    "index": len(self.outputs),
                    "name": link,
                    "object_type": "LINK",
                    "property": "QUALITY"
                })
    
    def generate_json(self, output_file: str, logging_level: str = "INFO") -> bool:
        """
        Generate JSON configuration file
        
        Args:
            output_file: Path to output JSON file
            logging_level: Logging level (OFF, ERROR, INFO, DEBUG)
        
        Returns:
            True if JSON was generated successfully, False otherwise
        """
        # Always add ElapsedTime as first input
        all_inputs = [
            {
                "index": 0,
                "name": "ElapsedTime",
                "object_type": "SYSTEM",
                "property": "ELAPSEDTIME"
            }
        ] + self.inputs
        
        # Calculate MD5 hash of .inp file
        inp_hash = self._calculate_file_hash(self.inp_file)
        
        # Create configuration dictionary
        config = {
            "version": "1.0",
            "logging_level": logging_level,
            "inp_file": os.path.basename(self.inp_file),
            "inp_file_hash": inp_hash,
            "hydraulic_timestep": self.parser.hydraulic_timestep,
            "_comment": f"IMPORTANT: Set GoldSim Basic Time Step to match hydraulic_timestep ({self.parser.hydraulic_timestep} seconds)",
            "input_count": len(all_inputs),
            "output_count": len(self.outputs),
            "inputs": all_inputs,
            "outputs": self.outputs
        }
        
        # Write JSON file
        try:
            with open(output_file, 'w') as f:
                json.dump(config, f, indent=2)
            return True
        except Exception as e:
            print(f"ERROR: Failed to write output file: {e}", file=sys.stderr)
            return False
    
    def _get_object_type(self, element_id: str) -> Optional[str]:
        """Determine object type for an element ID"""
        if element_id in self.parser.junctions or element_id in self.parser.reservoirs or element_id in self.parser.tanks:
            return "NODE"
        elif element_id in self.parser.pipes or element_id in self.parser.pumps or element_id in self.parser.valves:
            return "LINK"
        elif element_id in self.parser.patterns:
            return "PATTERN"
        else:
            return None
    
    def _is_valid_input_property(self, object_type: str, property_name: str) -> bool:
        """Check if property is valid for object type (inputs)"""
        valid_properties = {
            "NODE": ["DEMAND"],
            "LINK": ["STATUS", "SETTING"],
            "PATTERN": ["MULTIPLIER"]
        }
        return property_name in valid_properties.get(object_type, [])
    
    def _is_valid_output_property(self, object_type: str, property_name: str) -> bool:
        """Check if property is valid for object type (outputs)"""
        valid_properties = {
            "NODE": ["PRESSURE", "HEAD", "DEMAND", "TANKLEVEL", "QUALITY"],
            "LINK": ["FLOW", "VELOCITY", "HEADLOSS", "STATUS", "SETTING", "QUALITY"]
        }
        return property_name in valid_properties.get(object_type, [])
    
    def _calculate_file_hash(self, filename: str) -> str:
        """Calculate MD5 hash of file"""
        md5 = hashlib.md5()
        try:
            with open(filename, 'rb') as f:
                for chunk in iter(lambda: f.read(4096), b""):
                    md5.update(chunk)
            return md5.hexdigest()
        except Exception:
            return "unknown"


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description='Generate EPANET-GoldSim Bridge JSON configuration from EPANET .inp file',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Generate with default outputs (all tanks, junctions, links)
  python generate_mapping.py network.inp

  # Generate with custom inputs and outputs
  python generate_mapping.py network.inp --input TANK_1:DEMAND --output JUNCTION_5:PRESSURE

  # Generate with water quality outputs
  python generate_mapping.py network.inp --quality
        """
    )
    
    parser.add_argument('inp_file', help='Path to EPANET .inp file')
    parser.add_argument('--input', action='append', dest='inputs', metavar='ELEMENT:PROPERTY',
                       help='Add an input mapping (can be used multiple times)')
    parser.add_argument('--output', action='append', dest='outputs', metavar='ELEMENT:PROPERTY',
                       help='Add an output mapping (can be used multiple times)')
    parser.add_argument('--output-file', default='EpanetBridge.json',
                       help='Output JSON filename (default: EpanetBridge.json)')
    parser.add_argument('--quality', action='store_true',
                       help='Include water quality outputs for all nodes/links')
    parser.add_argument('--logging-level', default='INFO', choices=['OFF', 'ERROR', 'INFO', 'DEBUG'],
                       help='Set logging level (default: INFO)')
    
    args = parser.parse_args()
    
    # Parse EPANET .inp file
    print(f"Parsing EPANET model: {args.inp_file}")
    inp_parser = EpanetInpParser(args.inp_file)
    if not inp_parser.parse():
        return 1
    
    print(f"  Found {len(inp_parser.junctions)} junctions")
    print(f"  Found {len(inp_parser.reservoirs)} reservoirs")
    print(f"  Found {len(inp_parser.tanks)} tanks")
    print(f"  Found {len(inp_parser.pipes)} pipes")
    print(f"  Found {len(inp_parser.pumps)} pumps")
    print(f"  Found {len(inp_parser.valves)} valves")
    print(f"  Found {len(inp_parser.patterns)} patterns")
    print(f"  Hydraulic timestep: {inp_parser.hydraulic_timestep} seconds")
    print(f"  Water quality simulation: {'ENABLED' if inp_parser.quality_enabled else 'DISABLED'}")
    
    # Validate hydraulic timestep
    if inp_parser.hydraulic_timestep < 1:
        print(f"WARNING: Hydraulic timestep ({inp_parser.hydraulic_timestep}s) is less than 1 second", 
              file=sys.stderr)
    elif inp_parser.hydraulic_timestep > 3600:
        print(f"WARNING: Hydraulic timestep ({inp_parser.hydraulic_timestep}s) is greater than 1 hour", 
              file=sys.stderr)
    
    # Generate mapping
    generator = MappingGenerator(inp_parser, args.inp_file)
    
    # Add custom inputs
    if args.inputs:
        print(f"\nAdding {len(args.inputs)} custom inputs...")
        for input_spec in args.inputs:
            if not generator.add_input(input_spec):
                return 1
    
    # Add custom outputs or generate defaults
    if args.outputs:
        print(f"\nAdding {len(args.outputs)} custom outputs...")
        for output_spec in args.outputs:
            if not generator.add_output(output_spec):
                return 1
    else:
        print("\nGenerating default outputs (all tanks, junctions, links)...")
        generator.generate_default_outputs(include_quality=args.quality)
    
    # Warn if --quality flag is used but quality is not enabled in model
    if args.quality and not inp_parser.quality_enabled:
        print("\nWARNING: --quality flag specified but water quality simulation is not enabled in the EPANET model.", 
              file=sys.stderr)
        print("         Quality outputs will be included but will return zero values during simulation.", 
              file=sys.stderr)
        print("         To enable water quality, add 'Quality CHEMICAL' (or AGE/TRACE) to the [OPTIONS] section.", 
              file=sys.stderr)
    
    print(f"\nGenerated configuration:")
    print(f"  Inputs: {len(generator.inputs) + 1} (including ElapsedTime)")
    print(f"  Outputs: {len(generator.outputs)}")
    
    # Generate JSON file
    print(f"\nWriting configuration to: {args.output_file}")
    if not generator.generate_json(args.output_file, args.logging_level):
        return 1
    
    print("\nSUCCESS: Configuration file generated successfully!")
    print(f"\nIMPORTANT: Set GoldSim Basic Time Step to {inp_parser.hydraulic_timestep} seconds")
    
    return 0


if __name__ == '__main__':
    sys.exit(main())
