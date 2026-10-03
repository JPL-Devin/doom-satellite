// ======================================================================
// \title  DoomCoprocessorTopology.hpp
// \brief header file containing the topology instantiation definitions
// ======================================================================
#ifndef DOOMCOPROCESSOR_DOOMCOPROCESSORTOPOLOGY_HPP
#define DOOMCOPROCESSOR_DOOMCOPROCESSORTOPOLOGY_HPP
#include <DoomSatellite/DoomCoprocessor/Top/DoomCoprocessorTopologyDefs.hpp>

namespace DoomCoprocessor {

//! Base timer period: 70 Hz, divided down to the 35 Hz DOOM rate group and the 1 Hz housekeeping rate group
static constexpr U32 BASE_TIMER_USEC = 14286;
static constexpr U32 DOOM_RATE_DIVIDER = 2;
static constexpr U32 HOUSEKEEPING_RATE_DIVIDER = 70;
static constexpr U32 DOOM_TICK_USEC = BASE_TIMER_USEC * DOOM_RATE_DIVIDER;

/**
 * \brief initialize and run the F´ topology
 *
 * Initializes, configures, and starts the active components of the topology.
 *
 * @param state: object shuttling CLI arguments (hub addresses, WAD path) needed to construct the topology
 */
void setupTopology(const TopologyState& state);

/**
 * \brief run the rate groups until stopTopology is called
 *
 * Blocks the calling thread driving the base timer until stopTopology is called (e.g. from a signal handler).
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
