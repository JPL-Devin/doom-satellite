// ======================================================================
// \title  DoomFlightTopology.cpp
// \brief cpp file containing the topology instantiation code
//
// ======================================================================
// Provides access to autocoded functions
#include <DoomSatellite/DoomFlight/Top/DoomFlightTopologyAc.hpp>
// Note: Uncomment when using Svc:TlmPacketizer
// #include <DoomSatellite/DoomFlight/Top/DoomFlightPacketsAc.hpp>

// Necessary project-specified types
#include <Fw/Types/MallocAllocator.hpp>

#include <cstring>

// Allows easy reference to objects in FPP/autocoder required namespaces
using namespace DoomFlight;

// Instantiate a malloc allocator for cmdSeq buffer allocation
Fw::MallocAllocator mallocator;

constexpr FwSizeType BASE_RATEGROUP_PERIOD_MS = 1;  // 1Khz

// Helper function to calculate the period for a given rate group frequency
constexpr FwSizeType getRateGroupPeriod(const FwSizeType hz) {
    return 1000 / (hz * BASE_RATEGROUP_PERIOD_MS);
}

// The reference topology divides the incoming clock signal (1Hz) into sub-signals: 1Hz, 1/2Hz, and 1/4Hz with 0 offset
Svc::RateGroupDriver::DividerSet rateGroupDivisorsSet{{
    // Array of divider objects
    {getRateGroupPeriod(10), 0},  // 10Hz
    {getRateGroupPeriod(1), 0},   // 1Hz
}};

// Rate groups may supply a context token to each of the attached children whose purpose is set by the project. The
// reference topology sets each token to zero as these contexts are unused in this project.
U32 rateGroup10HzContext[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {getRateGroupPeriod(10)};
U32 rateGroup1HzContext[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {getRateGroupPeriod(1)};

enum TopologyConstants {
    HUB_BUFFER_MANAGER_ID = 300,
    HUB_BUFFER_SIZE = 1024,  // Matches the Drv::Udp default receive buffer size
    HUB_BUFFER_COUNT = 8,
    HUB_RECV_PRIORITY = 5,
    HUB_RECONNECT_PRIORITY = 6,
};

// Opcodes at or above the DoomCoprocessor base id are forwarded through the hub
constexpr FwOpcodeType REMOTE_BASE_OPCODE = 0x20000000;

/**
 * \brief configure/setup components in project-specific way
 *
 * This is a *helper* function which configures/sets up each component requiring project specific input. This includes
 * allocating resources, passing-in arguments, etc. This function may be inlined into the topology setup function if
 * desired, but is extracted here for clarity.
 */
void configureTopology(const TopologyState& state) {
    // Rate group driver needs a divisor list
    rateGroupDriver.configure(rateGroupDivisorsSet);
    // Rate groups require context arrays.
    rateGroup10Hz.configure(rateGroup10HzContext, FW_NUM_ARRAY_ELEMENTS(rateGroup10HzContext));
    rateGroup1Hz.configure(rateGroup1HzContext, FW_NUM_ARRAY_ELEMENTS(rateGroup1HzContext));
    // Reboot into the bootloader when the host opens the console at 1200 baud
    touchReset.configure(state.uartDevice);

    Svc::BufferManager::BufferBins hubBins;
    memset(&hubBins, 0, sizeof(hubBins));
    hubBins.bins[0].bufferSize = HUB_BUFFER_SIZE;
    hubBins.bins[0].numBuffers = HUB_BUFFER_COUNT;
    hubBufferManager.setup(HUB_BUFFER_MANAGER_ID, 0, mallocator, hubBins);

    (void)hubComDriver.configureSend(state.hubRemoteAddress, state.hubRemotePort);
    (void)hubComDriver.configureRecv("0.0.0.0", state.hubLocalPort);

    cmdSplitter.configure(REMOTE_BASE_OPCODE);
}

// Public functions for use in main program are namespaced with deployment name DoomFlight
namespace DoomFlight {
void setupTopology(const TopologyState& state) {
    // Autocoded initialization. Function provided by autocoder.
    initComponents(state);
    // Autocoded id setup. Function provided by autocoder.
    setBaseIds();
    // Autocoded connection wiring. Function provided by autocoder.
    connectComponents();
    // Autocoded command registration. Function provided by autocoder.
    regCommands();
    // Autocoded configuration. Function provided by autocoder.
    configComponents(state);
    // Project-specific component configuration. Function provided above. May be inlined, if desired.
    configureTopology(state);
    // Autocoded parameter loading. Function provided by autocoder.
    loadParameters();
    // Autocoded task kick-off (active components). Function provided by autocoder.
    startTasks(state);

    // UDP receive (and reconnect) tasks for the GenericHub link
    Os::TaskString hubName("hub");
    hubComDriver.start(hubName, HUB_RECV_PRIORITY, Default::STACK_SIZE, Os::Task::TASK_DEFAULT, HUB_RECONNECT_PRIORITY,
                       Default::STACK_SIZE);

    comDriver.configure(state.uartDevice, state.baudRate);
}

void startRateGroups() {
    timer.configure(BASE_RATEGROUP_PERIOD_MS);
    timer.start();
    while (1) {
        timer.cycle();
    }
}

void stopRateGroups() {
    timer.stop();
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
};  // namespace DoomFlight
