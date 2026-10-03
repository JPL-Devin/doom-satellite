module DoomCoprocessor {

  # ----------------------------------------------------------------------
  # Symbolic constants for port numbers
  # ----------------------------------------------------------------------

  enum Ports_RateGroups {
    rateGroupDoom
    rateGroup1Hz
  }

  @ Hub serial port carrying DoomFlight's echo of the DOOM telemetry stream (DoomFlight tlmSplitter.echoOut)
  constant HUB_TLM_ECHO_PORT = 0

  deployment topology DoomCoprocessor {

  # ----------------------------------------------------------------------
  # Subtopology instances
  # ----------------------------------------------------------------------
    instance DoomSubtopology.Subtopology

  # ----------------------------------------------------------------------
  # Instances used in the topology
  # ----------------------------------------------------------------------
    instance cmdDisp
    instance chronoTime
    instance hub
    instance hubComDriver
    instance hubByteStreamAdapter
    instance hubBufferManager
    instance rateGroupDoom
    instance rateGroup1Hz
    instance rateGroupDriver
    instance linuxTimer
    instance tlmEcho

  # ----------------------------------------------------------------------
  # Pattern graph specifiers
  # ----------------------------------------------------------------------

    command connections instance cmdDisp

    # Events go to the control node (DoomFlight) over the hub. The hub transport chain (hubByteStreamAdapter,
    # hubBufferManager, hubComDriver) is excluded: its events would re-enter the hub they report on.
    event connections instance hub {
      cmdDisp
      rateGroupDoom
      rateGroup1Hz
      tlmEcho
      DoomSubtopology.doom
      DoomSubtopology.doomBufferManager
      DoomSubtopology.frameDownsampler
      DoomSubtopology.frameTlmProcessor
    }

    # Only the DOOM engine and frame telemetry cross the hub: DoomFlight packetizes this stream with the merged
    # packet list (tools/merge_packets.py) and echoes it back to tlmEcho
    telemetry connections instance hub {
      DoomSubtopology.doom
      DoomSubtopology.frameTlmProcessor
    }

    time connections instance chronoTime

  # ----------------------------------------------------------------------
  # Telemetry packets
  # ----------------------------------------------------------------------

    include "DoomCoprocessorPackets.fppi"

  # ----------------------------------------------------------------------
  # Direct graph specifiers
  # ----------------------------------------------------------------------

    connections RateGroups {
      linuxTimer.CycleOut -> rateGroupDriver.CycleIn

      rateGroupDriver.CycleOut[Ports_RateGroups.rateGroupDoom] -> rateGroupDoom.CycleIn
      rateGroupDoom.RateGroupMemberOut[0] -> DoomSubtopology.Subtopology.schedIn

      rateGroupDriver.CycleOut[Ports_RateGroups.rateGroup1Hz] -> rateGroup1Hz.CycleIn
      rateGroup1Hz.RateGroupMemberOut[0] -> DoomSubtopology.Subtopology.bufferManagerSchedIn
      rateGroup1Hz.RateGroupMemberOut[1] -> tlmEcho.schedIn
    }

    connections HubCommands {
      hub.cmdDispOut -> cmdDisp.seqCmdBuff
      cmdDisp.seqCmdStatus -> hub.cmdRespIn
    }

    connections HubTelemetryEcho {
      hub.serialOut[HUB_TLM_ECHO_PORT] -> tlmEcho.serialIn
    }

    connections Hub {
      # Hub -> adapter -> UDP (send)
      hub.toBufferDriver                      -> hubByteStreamAdapter.bufferIn
      hubByteStreamAdapter.bufferInReturn     -> hub.toBufferDriverReturn
      hubByteStreamAdapter.toByteStreamDriver -> hubComDriver.$send

      # UDP -> adapter -> hub (receive)
      hubComDriver.$recv                              -> hubByteStreamAdapter.fromByteStreamDriver
      hubByteStreamAdapter.fromByteStreamDriverReturn -> hubComDriver.recvReturnIn
      hubByteStreamAdapter.bufferOut                  -> hub.fromBufferDriver
      hub.fromBufferDriverReturn                      -> hubByteStreamAdapter.bufferOutReturn
      hubComDriver.ready                              -> hubByteStreamAdapter.byteStreamDriverReady

      # Shared buffer allocation
      hub.allocate            -> hubBufferManager.bufferGetCallee
      hub.deallocate          -> hubBufferManager.bufferSendIn
      hubComDriver.allocate   -> hubBufferManager.bufferGetCallee
      hubComDriver.deallocate -> hubBufferManager.bufferSendIn
    }
  }
}
