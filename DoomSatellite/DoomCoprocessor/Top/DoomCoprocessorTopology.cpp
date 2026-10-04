// ======================================================================
// \title  DoomCoprocessorTopology.cpp
// \brief cpp file containing the topology instantiation code
// ======================================================================
// Provides access to autocoded functions
#include <DoomSatellite/DoomCoprocessor/Top/DoomCoprocessorTopology.hpp>
#include <DoomSatellite/DoomCoprocessor/Top/DoomCoprocessorTopologyAc.hpp>

#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/MallocAllocator.hpp>
#include <Os/Task.hpp>

#include <cstring>

// Allows easy reference to objects in FPP/autocoder required namespaces
using namespace DoomCoprocessor;

namespace {
Fw::MallocAllocator mallocator;

enum TopologyConstants {
    HUB_BUFFER_MANAGER_ID = 300,
    HUB_SMALL_BUFFER_SIZE = 1024,  // Commands, events, telemetry and packed palettes
    HUB_SMALL_BUFFER_COUNT = 16,
    HUB_LARGE_BUFFER_SIZE = 4096,  // UDP receive buffers and packed frames (4,009 bytes, 4,025 with the hub header)
    HUB_LARGE_BUFFER_COUNT = 32,
    HUB_RECV_PRIORITY = 30,
    HUB_RECONNECT_PRIORITY = 29,
};

const Svc::RateGroupDriver::DividerSet rateGroupDivisorsSet{
    {{DOOM_RATE_DIVIDER, 0}, {HOUSEKEEPING_RATE_DIVIDER, 0}}};

Svc::ActiveRateGroup::ContextArray rateGroupDoomContext(0U);
Svc::ActiveRateGroup::ContextArray rateGroup1HzContext(0U);

void configureTopology(const DoomCoprocessor::TopologyState& state) {
    rateGroupDriver.configure(rateGroupDivisorsSet);
    // DoomEngine reads the tick period from its schedIn context
    rateGroupDoomContext[0] = DOOM_TICK_USEC;
    rateGroupDoom.configure(rateGroupDoomContext);
    rateGroup1Hz.configure(rateGroup1HzContext);

    Svc::BufferManager::BufferBins hubBins;
    memset(&hubBins, 0, sizeof(hubBins));
    hubBins.bins[0].bufferSize = HUB_SMALL_BUFFER_SIZE;
    hubBins.bins[0].numBuffers = HUB_SMALL_BUFFER_COUNT;
    hubBins.bins[1].bufferSize = HUB_LARGE_BUFFER_SIZE;
    hubBins.bins[1].numBuffers = HUB_LARGE_BUFFER_COUNT;
    hubBufferManager.setup(HUB_BUFFER_MANAGER_ID, 0, mallocator, hubBins);

    (void)hubComDriver.configureSend(state.hubRemoteAddress.toChar(), state.hubRemotePort);
    (void)hubComDriver.configureRecv("0.0.0.0", state.hubLocalPort, HUB_LARGE_BUFFER_SIZE);

    // All engine heap allocation happens here, before any task runs. An unreadable WAD leaves the engine uncreated
    // and doom.Start is rejected.
    Doom::InitStatus initStatus = DoomSubtopology::doom.setWadPath(state.wadPath.toChar());
    if (initStatus == Doom::InitStatus::OK) {
        initStatus = DoomSubtopology::doom.initEngine();
    }
    if (initStatus != Doom::InitStatus::OK) {
        Fw::String text;
        initStatus.toString(text);
        Fw::Logger::log("DOOM engine init failed (%s): Start will be rejected\n", text.toChar());
    }
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

    if (state.autoStart) {
        Fw::String status;
        DoomSubtopology::doom.forceStart().toString(status);
        Fw::Logger::log("Auto-start: doom.forceStart() returned %s\n", status.toChar());
    }
}

void runTopology() {
    linuxTimer.startTimer(Fw::TimeInterval(0, BASE_TIMER_USEC));
}

void stopTopology() {
    linuxTimer.quit();
}

void teardownTopology(const TopologyState& state) {
    // Autocoded (active component) task clean-up. Functions provided by topology autocoder.
    stopTasks(state);
    freeThreads(state);
    hubComDriver.stop();
    (void)hubComDriver.join();
    hubBufferManager.cleanup();
    tearDownComponents(state);
    // Releases the Python mirror objects held by fprime-python components before the interpreter finalizes
    deinitComponents(state);
}
};  // namespace DoomCoprocessor
