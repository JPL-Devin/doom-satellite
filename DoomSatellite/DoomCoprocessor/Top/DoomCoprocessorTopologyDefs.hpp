// ======================================================================
// \title  DoomCoprocessorTopologyDefs.hpp
// \brief required header file containing the required definitions for the topology autocoder
// ======================================================================
#ifndef DOOMCOPROCESSOR_DOOMCOPROCESSORTOPOLOGYDEFS_HPP
#define DOOMCOPROCESSOR_DOOMCOPROCESSORTOPOLOGYDEFS_HPP

// Include autocoded FPP constants
#include "DoomSatellite/DoomCoprocessor/Top/FppConstantsAc.hpp"

#include "Doom/DoomConfig/FppConstantsAc.hpp"
#include "Doom/DoomSubtopology/SubtopologyTopologyDefs.hpp"
#include "Fw/Types/String.hpp"

// DoomSubtopology configuration phases call memset
#include <cstring>

/**
 * \brief required ping constants
 *
 * The topology autocoder requires a WARN and FATAL constant definition for each component that supports the health-ping
 * interface when a health component is in the topology. This topology has no health component.
 */
namespace PingEntries {}  // namespace PingEntries

// Definitions are placed within a namespace named after the deployment
namespace DoomCoprocessor {

/**
 * \brief required type definition to carry state
 *
 * The topology autocoder requires an object that carries state with the name `DoomCoprocessor::TopologyState`. Only
 * the type definition is required by the autocoder and the contents of this object are otherwise opaque to the
 * autocoder.
 */
struct TopologyState {
    Fw::String hubRemoteAddress;             //!< IPv4 address of the remote (control node) GenericHub deployment
    U16 hubRemotePort = 0;                   //!< UDP port the remote GenericHub receives on
    U16 hubLocalPort = 0;                    //!< UDP port this GenericHub receives on
    Fw::String wadPath;                      //!< DOOM IWAD path; the engine is not created when it cannot be opened
    bool autoStart = false;                  //!< Start the DOOM engine at setup instead of waiting for doom.Start
    DoomSubtopology::SubtopologyState doom;
};

namespace PingEntries = ::PingEntries;
}  // namespace DoomCoprocessor
#endif
