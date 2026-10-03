// ======================================================================
// \title  Main.cpp
// \brief main program for the F' application. Intended for CLI-based systems (Linux, macOS)
//
// ======================================================================
// Used to access topology functions
#include <DoomSatellite/DoomFlight/Top/DoomFlightTopology.hpp>
#include <Fw/Types/Assert.hpp>
#include <Os/Os.hpp>

// Zephyr headers follow F Prime headers: Zephyr's EMPTY macro collides with Os::Queue::Status::EMPTY
#include <cmsis_core.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/fatal.h>
#include <zephyr/kernel.h>
#if defined(CONFIG_BOARD_TEENSY41)
#include <zephyr/net/phy.h>
#endif
#if defined(CONFIG_RETENTION_BOOT_MODE)
#include <zephyr/retention/bootmode.h>
#endif
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>

#include <cstring>

const struct device* serial = DEVICE_DT_GET(DT_NODELABEL(cdc_acm_uart0));

//! Seconds the CDC ACM port is watched for a 1200 baud touch before the topology starts
static constexpr U32 BOOT_WINDOW_SECONDS = 10;
static constexpr U32 CRASH_RECORD_MAGIC = 0xDEADD00Du;

//! Kernel fatal error state, kept across the warm reboot that follows the fatal error
struct CrashRecord {
    U32 magic;
    U32 reason;
    U32 pc;
    U32 lr;
    U32 xpsr;
    U32 cfsr;
    U32 hfsr;
    U32 mmfar;
    U32 bfar;
    U32 stackStart;
    char thread[32];
};
static __noinit CrashRecord crashRecord;

//! Enters the board bootloader when the host has set the CDC ACM port to 1200 baud
static void touchResetCheck() {
    U32 baud = 0;
    if ((uart_line_ctrl_get(serial, UART_LINE_CTRL_BAUD_RATE, &baud) == 0) && (baud == 1200)) {
#if defined(CONFIG_RETENTION_BOOT_MODE)
        (void)bootmode_set(BOOT_MODE_TYPE_BOOTLOADER);
#endif
#if defined(CONFIG_BOARD_TEENSY41)
        // Teensy bootloader chip halts the core on this breakpoint and enters HalfKay
        __asm__ volatile("bkpt #251");
#endif
        sys_reboot(SYS_REBOOT_WARM);
    }
}

extern "C" void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf* esf) {
    crashRecord.reason = reason;
    crashRecord.pc = (esf != nullptr) ? esf->basic.pc : 0;
    crashRecord.lr = (esf != nullptr) ? esf->basic.lr : 0;
    crashRecord.xpsr = (esf != nullptr) ? esf->basic.xpsr : 0;
#if defined(CONFIG_ARMV7_M_ARMV8_M_MAINLINE)
    crashRecord.cfsr = SCB->CFSR;
    crashRecord.hfsr = SCB->HFSR;
    crashRecord.mmfar = SCB->MMFAR;
    crashRecord.bfar = SCB->BFAR;
#else
    crashRecord.cfsr = 0;
    crashRecord.hfsr = 0;
    crashRecord.mmfar = 0;
    crashRecord.bfar = 0;
#endif
    struct k_thread* thread = k_current_get();
    crashRecord.stackStart = (thread != nullptr) ? static_cast<U32>(thread->stack_info.start) : 0;
    const char* name = (thread != nullptr) ? k_thread_name_get(thread) : nullptr;
    (void)strncpy(crashRecord.thread, (name != nullptr) ? name : "", sizeof(crashRecord.thread) - 1);
    crashRecord.thread[sizeof(crashRecord.thread) - 1] = '\0';
    crashRecord.magic = CRASH_RECORD_MAGIC;
    sys_reboot(SYS_REBOOT_WARM);
}

static void printCrashRecord(const CrashRecord& record, U32 resetCause) {
    printk("DoomFlight: reset cause 0x%08x\n", resetCause);
    printk("DoomFlight: FATAL reason %u thread '%s' stack 0x%08x\n", record.reason, record.thread, record.stackStart);
    printk("DoomFlight: pc 0x%08x lr 0x%08x xpsr 0x%08x\n", record.pc, record.lr, record.xpsr);
    printk("DoomFlight: cfsr 0x%08x hfsr 0x%08x mmfar 0x%08x bfar 0x%08x\n", record.cfsr, record.hfsr, record.mmfar,
           record.bfar);
}

#if defined(CONFIG_BOARD_TEENSY41)
static const struct device* const ethernetPhy = DEVICE_DT_GET(DT_NODELABEL(phy));

//! Reapplies the DP83825 configuration (RMII reference clock, advertised link modes) once the ENET MAC is running
static void configureEthernetPhy() {
    const int status = phy_configure_link(
        ethernetPhy,
        static_cast<enum phy_link_speed>(LINK_HALF_10BASE | LINK_FULL_10BASE | LINK_HALF_100BASE | LINK_FULL_100BASE),
        static_cast<enum phy_cfg_link_flag>(0));
    if (status != 0) {
        printk("DoomFlight: PHY configuration failed %d\n", status);
    }
}
#endif

//! Watches for the 1200 baud touch before the topology starts. After a fatal error, stays here reporting it.
static void bootWindow() {
    U32 resetCause = 0;
    (void)hwinfo_get_reset_cause(&resetCause);
    (void)hwinfo_clear_reset_cause();
#if defined(CONFIG_BOARD_TEENSY41)
    configureEthernetPhy();
#endif
    const bool crashed = (crashRecord.magic == CRASH_RECORD_MAGIC);
    const CrashRecord record = crashRecord;
    crashRecord.magic = 0;
    for (U32 tick = 0; crashed || (tick < (BOOT_WINDOW_SECONDS * 10)); tick++) {
        touchResetCheck();
        if ((tick % 10) == 0) {
            if (crashed) {
                printCrashRecord(record, resetCause);
            } else {
                printk("DoomFlight: boot window %us, reset cause 0x%08x\n", BOOT_WINDOW_SECONDS - (tick / 10),
                       resetCause);
            }
        }
        k_sleep(K_MSEC(100));
    }
}

//! Holds the asserting thread instead of rebooting so the assert message reaches the console and the
//! 1200 baud touch can still reach the bootloader
class ParkingAssertHook : public Fw::AssertHook {
  public:
    void printAssert(const CHAR* msg) override { printk("%s\n", msg); }

    void doAssert() override {
        while (true) {
            touchResetCheck();
            k_sleep(K_MSEC(100));
        }
    }
};

ParkingAssertHook assertHook;

int main(int argc, char* argv[]) {
    // ** DO NOT REMOVE **//
    //
    // This wait is necessary to allow the USB CDC ACM interface to initialize before
    // the application starts writing to it.
    bootWindow();

    assertHook.registerHook();
    Os::init();
    // Object for communicating state to the topology
    DoomFlight::TopologyState inputs;
    inputs.uartDevice = serial;
    inputs.baudRate = 115200;
    inputs.hubRemoteAddress = "192.168.11.1";
    inputs.hubRemotePort = 50555;
    inputs.hubLocalPort = 50556;

    // Setup, cycle, and teardown topology
    printk("DoomFlight: setting up topology\n");
    DoomFlight::setupTopology(inputs);
    printk("DoomFlight: starting rate groups\n");
    DoomFlight::startRateGroups();  // Program loop
    DoomFlight::teardownTopology(inputs);
    return 0;
}
