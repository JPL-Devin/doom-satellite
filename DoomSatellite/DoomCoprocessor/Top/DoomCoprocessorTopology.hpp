// ======================================================================
// \title  DoomCoprocessorTopology.hpp
// \brief header file containing the topology instantiation definitions
// ======================================================================
#ifndef DOOMCOPROCESSOR_DOOMCOPROCESSORTOPOLOGY_HPP
#define DOOMCOPROCESSOR_DOOMCOPROCESSORTOPOLOGY_HPP
// Included for access to DoomCoprocessor::TopologyState and DoomCoprocessor::ConfigObjects::pingEntries.
#include <DoomSatellite/DoomCoprocessor/Top/DoomCoprocessorTopologyDefs.hpp>

namespace DoomCoprocessor {
/**
 * \brief initialize and run the F´ topology
 *
 * Initializes, configures, and starts the active components of the topology.
 *
 * @param state: object shuttling CLI arguments (hostname, port) needed to construct the topology
 */
void setupTopology(const TopologyState& state);

/**
 * \brief run the topology until stopTopology is called
 *
 * Blocks the calling thread until stopTopology is called (e.g. from a signal handler).
 */
void runTopology();

/**
 * \brief request that runTopology return
 *
 * Safe to call from a signal handler.
 */
void stopTopology();

/**
 * \brief stop and clean up the topology
 *
 * Stops the active components' tasks and releases their resources.
 *
 * @param state: state object used to construct the topology
 */
void teardownTopology(const TopologyState& state);
}  // namespace DoomCoprocessor
#endif
