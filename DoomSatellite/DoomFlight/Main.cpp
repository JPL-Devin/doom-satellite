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
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/retention/bootmode.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>

const struct device* serial = DEVICE_DT_GET(DT_NODELABEL(cdc_acm_uart0));

//! Holds the asserting thread instead of rebooting so the assert message reaches the console and the
//! 1200 baud touch can still reach the bootloader
class ParkingAssertHook : public Fw::AssertHook {
  public:
    void printAssert(const CHAR* msg) override { printk("%s\n", msg); }

    void doAssert() override {
        while (true) {
            U32 baud = 0;
            if ((uart_line_ctrl_get(serial, UART_LINE_CTRL_BAUD_RATE, &baud) == 0) && (baud == 1200)) {
                (void)bootmode_set(BOOT_MODE_TYPE_BOOTLOADER);
                sys_reboot(SYS_REBOOT_WARM);
            }
            k_sleep(K_MSEC(100));
        }
    }
};

ParkingAssertHook assertHook;

int main(int argc, char* argv[]) {
    // ** DO NOT REMOVE **//
    //
    // This sleep is necessary to allow the USB CDC ACM interface to initialize before
    // the application starts writing to it.
    k_sleep(K_MSEC(3000));

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
