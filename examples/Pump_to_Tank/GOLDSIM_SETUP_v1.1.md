# GoldSim Setup for Pump_to_Tank Example (v1.1)

This guide shows how to set up the Pump_to_Tank example in GoldSim with the new POWER and EFFICIENCY outputs.

## Configuration Changes in v1.1

The `EpanetBridge.json` file now includes 7 outputs (previously 5):

- Output 0: Tank1 HEAD (ft)
- Output 1: J2 PRESSURE (psi)
- Output 2: Pump1 FLOW (GPM)
- Output 3: Pipe 1 FLOW (GPM) - flow out of tank
- Output 4: Pipe 2 FLOW (GPM) - flow into tank
- **Output 5: Pump1 POWER (HP)** ← NEW
- **Output 6: Pump1 EFFICIENCY (%)** ← NEW

## GoldSim External Element Setup

### Inputs (3 total)
1. **Input 1**: Link to `ETime` (GoldSim's elapsed time in seconds)
2. **Input 2**: Pump1 setting (0.0 = off, 1.0 = on/full speed)
3. **Input 3**: Outlet emitter coefficient (controls tank outflow)

### Outputs (7 total)
1. **Output 1**: Tank1 hydraulic head (ft)
2. **Output 2**: J2 pressure (psi)
3. **Output 3**: Pump1 flow (GPM)
4. **Output 4**: Pipe 1 flow out of tank (GPM)
5. **Output 5**: Pipe 2 flow into tank (GPM)
6. **Output 6**: Pump1 power rating (HP)
7. **Output 7**: Pump1 efficiency (%)

## Visualizing Pump Properties in GoldSim

### Power Output
- Create a **Result** element connected to External Output 6
- Units: Horsepower (HP)
- Computed from flow rate, head gain, and efficiency
- Will show actual pump power consumption (e.g., ~6.6 HP at 100 GPM)
- Shows 0 HP when pump is off

### Efficiency Output
- Create a **Result** element connected to External Output 7
- Units: Percent (%) - but EPANET returns as fraction
- Expected values:
  - When pump is ON: ~0.75 (which is 75% expressed as fraction)
  - When pump is OFF: 0.0
- Multiply by 100 in GoldSim to display as percentage (75%)

### Creating Time History Plots

To visualize pump performance over time:

1. **Pump Efficiency vs Time**
   - X-axis: Elapsed Time
   - Y-axis: Pump1 Efficiency (%)
   - Shows when pump is on/off and operating efficiency

2. **Pump Flow vs Efficiency**
   - X-axis: Pump1 Flow (GPM)
   - Y-axis: Pump1 Efficiency (%)
   - Shows pump operating point on efficiency curve

3. **Combined Dashboard**
   - Tank Level (ft)
   - Pump Status (on/off)
   - Pump Flow (GPM)
   - Pump Efficiency (%)
   - All on same time axis

## Important Notes

### POWER Property
- Computed from flow rate, head gain, and efficiency
- Formula: Power (HP) = (Flow × Head × SG) / (3960 × Efficiency)
- Returns actual pump power consumption in horsepower
- Returns 0 HP when pump is off or not flowing

### EFFICIENCY Property
- Returns current computed efficiency based on operating point
- Calculated from pump curve and current flow/head
- Global efficiency setting in .inp file affects this value
- Returns 0% when pump is off
- Note: EPANET returns efficiency as a fraction (0.75 = 75%)

### Timestep Synchronization
- Ensure GoldSim's **Basic Time Step** matches `hydraulic_timestep` in JSON (300 seconds = 5 minutes)
- Mismatched timesteps will cause incorrect results

## Example GoldSim Model Structure

```
[Reservoir] → [Pump1] → [J2] → [Pipe 2] → [Tank1] → [Pipe 1] → [Outlet]
                ↑                                                    ↑
         Control from                                        Emitter
         GoldSim                                            (outflow)
```

### Control Logic Example
```
IF Tank1_Head < 205 ft THEN
    Pump1_Setting = 1.0  (ON)
ELSE IF Tank1_Head > 215 ft THEN
    Pump1_Setting = 0.0  (OFF)
ELSE
    Pump1_Setting = Previous_Value  (maintain state)
END IF
```

## Troubleshooting

**Efficiency shows 0.75% instead of 75%**
- This is correct - EPANET returns efficiency as a fraction (0.75 = 75%)
- The global efficiency in the .inp file is 75, which EPANET interprets as 75%
- The API returns this as 0.75 (fraction)
- You can multiply by 100 in GoldSim if you want to display as 75%

**Power values look reasonable**
- Power is computed from: Power (HP) = (Flow × Head × SG) / (3960 × Efficiency)
- Example: 101 GPM × 194 ft / (3960 × 0.75) ≈ 6.6 HP
- Power will vary with flow rate and operating point on pump curve
- Power = 0 when pump is off

**Efficiency doesn't change with flow**
- Check that pump is actually running (setting = 1.0)
- Verify pump curve is defined correctly in .inp file
- Check global efficiency setting in [ENERGY] section

## Files Required

Place these files in the same folder as your `.gsm` file:
- `gs_epanet.dll` (v1.1 or later)
- `epanet2.dll`
- `EpanetBridge.json` (updated for v1.1)
- `Pump_to_Tank.inp`

---

**Version**: 1.1  
**Last Updated**: February 9, 2026
