// ======================================================================
// \title  DoomFlightTopology.cpp
// \brief cpp file containing the topology instantiation code
//
// ======================================================================
// Provides access to autocoded functions
#include <DoomSatellite/DoomFlight/Top/DoomFlightTopologyAc.hpp>

// Necessary project-specified types
#include <Fw/Types/MallocAllocator.hpp>

#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

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
    HUB_SMALL_BUFFER_SIZE = 1024,  // Commands, events, telemetry and packed palettes
    HUB_SMALL_BUFFER_COUNT = 8,
    HUB_LARGE_BUFFER_SIZE = 4096,  // UDP receive buffers and packed frames (4,009 bytes, 4,025 with the hub header)
    HUB_LARGE_BUFFER_COUNT = 12,
    HUB_RECV_PRIORITY = 5,
    HUB_RECONNECT_PRIORITY = 6,
};

// Opcodes at or above the DoomCoprocessor base id are forwarded through the hub
constexpr FwOpcodeType REMOTE_BASE_OPCODE = 0x20000000;

#if defined(CONFIG_USBD_CDC_ACM_CLASS)
constexpr U32 CDC_FLOW_CONTROL_POLL_MS = 100;
static const struct device* cdcUartDevice = nullptr;
static struct k_work_delayable cdcFlowControlWork;

//! CDC ACM discards writes once its TX FIFO is full unless flow control is set, in which case writes block. Writes
//! block only while the host asserts DTR, so a closed host port drops downlink bytes instead of stalling the sender.
static void updateCdcFlowControl(struct k_work* work) {
    U32 dtr = 0;
    struct uart_config uartConfig;
    if ((uart_line_ctrl_get(cdcUartDevice, UART_LINE_CTRL_DTR, &dtr) == 0) &&
        (uart_config_get(cdcUartDevice, &uartConfig) == 0)) {
        uartConfig.flow_ctrl = (dtr != 0) ? UART_CFG_FLOW_CTRL_RTS_CTS : UART_CFG_FLOW_CTRL_NONE;
        (void)uart_configure(cdcUartDevice, &uartConfig);
    }
    (void)k_work_schedule(k_work_delayable_from_work(work), K_MSEC(CDC_FLOW_CONTROL_POLL_MS));
}
#endif

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
    // Reboot into the bootloader when the host opens the console at the board's touch baud rate
    if (touchReset.configure(state.uartDevice) != Fw::Success::SUCCESS) {
        printk("Touch reset unavailable\n");
    }

    Svc::BufferManager::BufferBins hubBins;
    memset(&hubBins, 0, sizeof(hubBins));
    hubBins.bins[0].bufferSize = HUB_SMALL_BUFFER_SIZE;
    hubBins.bins[0].numBuffers = HUB_SMALL_BUFFER_COUNT;
    hubBins.bins[1].bufferSize = HUB_LARGE_BUFFER_SIZE;
    hubBins.bins[1].numBuffers = HUB_LARGE_BUFFER_COUNT;
    hubBufferManager.setup(HUB_BUFFER_MANAGER_ID, 0, ComCcsds::Allocation::memAllocator, hubBins);

    (void)hubComDriver.configureSend(state.hubRemoteAddress, state.hubRemotePort);
    (void)hubComDriver.configureRecv("0.0.0.0", state.hubLocalPort, HUB_LARGE_BUFFER_SIZE);

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
#if defined(CONFIG_USBD_CDC_ACM_CLASS)
    cdcUartDevice = state.uartDevice;
    k_work_init_delayable(&cdcFlowControlWork, updateCdcFlowControl);
    (void)k_work_schedule(&cdcFlowControlWork, K_NO_WAIT);
#endif
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
