// ======================================================================
// \title  Main.cpp
// \brief main program for the DoomCoprocessor F' application (arm-linux)
// ======================================================================
// Used to access topology functions
#include <DoomSatellite/DoomCoprocessor/Top/DoomCoprocessorTopology.hpp>
#include <Os/Os.hpp>
// Used for signal handling shutdown
#include <signal.h>
// Used for printf functions
#include <cstdio>
#include <cstdlib>

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
    inputs.hubRemoteAddress = (argc > 1) ? argv[1] : "192.168.11.2";
    inputs.hubRemotePort = static_cast<U16>((argc > 2) ? std::atoi(argv[2]) : 50556);
    inputs.hubLocalPort = static_cast<U16>((argc > 3) ? std::atoi(argv[3]) : 50555);

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
