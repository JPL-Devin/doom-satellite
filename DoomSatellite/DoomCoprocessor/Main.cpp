// ======================================================================
// \title  Main.cpp
// \brief main program for the DoomCoprocessor F' application (arm-linux)
//
// Usage:
//   DoomCoprocessor [-a remote_address] [-p remote_port] [-u local_port] [-w /path/to/DOOM1.WAD] [-S] [-h]
// ======================================================================
// Used to access topology functions
#include <DoomSatellite/DoomCoprocessor/Top/DoomCoprocessorTopology.hpp>
#include <Fw/Logger/Logger.hpp>
#include <Os/Os.hpp>
// Used for signal handling shutdown
#include <signal.h>
#include <unistd.h>
// Used for printf functions
#include <cstdio>
#include <cstdlib>

namespace {
const char* const DEFAULT_REMOTE_ADDRESS = "192.168.11.2";
constexpr U16 DEFAULT_REMOTE_PORT = 50556;
constexpr U16 DEFAULT_LOCAL_PORT = 50555;
const char* const DEFAULT_WAD_PATH = "doom1.wad";

void printUsage(const char* app) {
    Fw::Logger::log(
        "Usage: %s [-a address] [-p port] [-u port] [-w wad_path] [-S] [-h]\n"
        "    -a address   Control node (DoomFlight) hub address (default %s)\n"
        "    -p port      Control node hub UDP port (default %u)\n"
        "    -u port      Local hub UDP port (default %u)\n"
        "    -w wad_path  DOOM IWAD path (default %s)\n"
        "    -S           Auto-start the DOOM engine on boot\n"
        "    -h           Print this usage text and exit\n",
        app, DEFAULT_REMOTE_ADDRESS, DEFAULT_REMOTE_PORT, DEFAULT_LOCAL_PORT, DEFAULT_WAD_PATH);
}
}  // namespace

/**
 * \brief shutdown topology on signal
 *
 * The topology runs until stopped via a signal such that it is performed via Ctrl-C.
 *
 * @param signum
 */
static void signalHandler(int signum) {
    DoomCoprocessor::stopTopology();
}

int main(int argc, char* argv[]) {
    Os::init();
    // Object for communicating state to the topology
    DoomCoprocessor::TopologyState inputs;
    inputs.hubRemoteAddress = DEFAULT_REMOTE_ADDRESS;
    inputs.hubRemotePort = DEFAULT_REMOTE_PORT;
    inputs.hubLocalPort = DEFAULT_LOCAL_PORT;
    inputs.wadPath = DEFAULT_WAD_PATH;

    int option = 0;
    while ((option = getopt(argc, argv, "ha:p:u:w:S")) != -1) {
        switch (option) {
            case 'a':
                inputs.hubRemoteAddress = optarg;
                break;
            case 'p':
                inputs.hubRemotePort = static_cast<U16>(std::atoi(optarg));
                break;
            case 'u':
                inputs.hubLocalPort = static_cast<U16>(std::atoi(optarg));
                break;
            case 'w':
                inputs.wadPath = optarg;
                break;
            case 'S':
                inputs.autoStart = true;
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }

    // Setup program shutdown via Ctrl-C
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    (void)printf("Hit Ctrl-C to quit\n");

    // Setup, run, and teardown topology
    DoomCoprocessor::setupTopology(inputs);
    DoomCoprocessor::runTopology();
    DoomCoprocessor::teardownTopology(inputs);
    (void)printf("Exiting...\n");
    return 0;
}
