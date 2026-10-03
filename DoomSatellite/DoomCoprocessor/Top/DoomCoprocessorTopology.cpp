// ======================================================================
// \title  DoomCoprocessorTopology.cpp
// \brief cpp file containing the topology instantiation code
// ======================================================================
// Provides access to autocoded functions
#include <DoomSatellite/DoomCoprocessor/Top/DoomCoprocessorTopology.hpp>
#include <DoomSatellite/DoomCoprocessor/Top/DoomCoprocessorTopologyAc.hpp>

#include <Fw/Types/MallocAllocator.hpp>
#include <Os/Task.hpp>

#include <atomic>
#include <cstring>

// Allows easy reference to objects in FPP/autocoder required namespaces
using namespace DoomCoprocessor;

namespace {
std::atomic<bool> g_running(true);
constexpr FwSizeType RUN_POLL_MS = 100;
Fw::MallocAllocator mallocator;

enum TopologyConstants {
    HUB_BUFFER_MANAGER_ID = 300,
    HUB_BUFFER_SIZE = 1024,
    HUB_BUFFER_COUNT = 16,
    HUB_RECV_PRIORITY = 30,
    HUB_RECONNECT_PRIORITY = 29,
};

void configureTopology(const DoomCoprocessor::TopologyState& state) {
    Svc::BufferManager::BufferBins hubBins;
    memset(&hubBins, 0, sizeof(hubBins));
    hubBins.bins[0].bufferSize = HUB_BUFFER_SIZE;
    hubBins.bins[0].numBuffers = HUB_BUFFER_COUNT;
    hubBufferManager.setup(HUB_BUFFER_MANAGER_ID, 0, mallocator, hubBins);

    (void)hubComDriver.configureSend(state.hubRemoteAddress, state.hubRemotePort);
    (void)hubComDriver.configureRecv("0.0.0.0", state.hubLocalPort);
}
}  // namespace

// Public functions for use in main program are namespaced with deployment name DoomCoprocessor
namespace DoomCoprocessor {
void setupTopology(const TopologyState& state) {
    // Autocoded initialization. Function provided by autocoder.
    initComponents(state);
    // Autocoded id setup. Function provided by autocoder.
    setBaseIds();
    // Autocoded connection wiring. Function provided by autocoder.
    connectComponents();
    // Project-specific configuration. Hub buffers must exist before command registration emits events.
    configureTopology(state);
    // Autocoded command registration. Function provided by autocoder.
    regCommands();
    // Autocoded configuration. Function provided by autocoder.
    configComponents(state);
    // Autocoded parameter loading. Function provided by autocoder.
    loadParameters();
    // Autocoded task kick-off (active components). Function provided by autocoder.
    startTasks(state);

    // UDP receive (and reconnect) tasks for the GenericHub link
    Os::TaskString hubName("hub");
    hubComDriver.start(hubName, HUB_RECV_PRIORITY, Default::STACK_SIZE, Os::Task::TASK_DEFAULT, HUB_RECONNECT_PRIORITY,
                       Default::STACK_SIZE);
}

void runTopology() {
    while (g_running.load()) {
        Os::Task::delay(Fw::TimeInterval(0, RUN_POLL_MS * 1000));
    }
}

void stopTopology() {
    g_running.store(false);
}

void teardownTopology(const TopologyState& state) {
    // Autocoded (active component) task clean-up. Functions provided by topology autocoder.
    stopTasks(state);
    freeThreads(state);
    hubComDriver.stop();
    (void)hubComDriver.join();
    hubBufferManager.cleanup();
    tearDownComponents(state);
}
};  // namespace DoomCoprocessor
