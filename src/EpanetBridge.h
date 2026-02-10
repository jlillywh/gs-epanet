/**
 * @file EpanetBridge.h
 * @brief Header for EPANET-GoldSim Bridge DLL
 * 
 * This header declares the main entry point for the GoldSim External Function
 * interface. The bridge enables real-time coupling between GoldSim dynamic
 * simulation software and the EPA EPANET hydraulic modeling engine.
 * 
 * Requirements: 1.1
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Main entry point for GoldSim External Function
 * 
 * This function implements the GoldSim External Function API and handles
 * all communication between GoldSim and EPANET.
 * 
 * @param methodID The operation to perform (0=INITIALIZE, 1=CALCULATE, 2=REP_VERSION, 3=REP_ARGUMENTS, 99=CLEANUP)
 * @param status Pointer to status code to return (0=SUCCESS, 1=FAILURE, -1=FAILURE_WITH_MSG, -2=INCREASE_MEMORY, 99=CLEANUP_NOW)
 * @param inargs Input array from GoldSim (read-only, must not be modified)
 * @param outargs Output array to GoldSim (write results here)
 */
void __declspec(dllexport) EpanetBridge(int methodID, int* status, double* inargs, double* outargs);

#ifdef __cplusplus
}
#endif
