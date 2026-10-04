module DoomCoprocessor {

  # ----------------------------------------------------------------------
  # Symbolic constants for port numbers
  # ----------------------------------------------------------------------

  enum Ports_RateGroups {
    rateGroupDoom
    rateGroup1Hz
  }

  @ Hub buffer port carrying packed downsampled frames and palettes to DoomFlight, and DoomFlight's echo of them back
  constant HUB_FRAME_PORT = 0

  deployment topology DoomCoprocessor {

  # ----------------------------------------------------------------------
  # DoomSubtopology instances: the engine and downsampler run here, FrameTlmProcessor runs on DoomFlight
  # ----------------------------------------------------------------------
    instance DoomSubtopology.doom
    instance DoomSubtopology.doomBufferManager
    instance DoomSubtopology.frameDownsampler

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
    instance frameAdapter

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
      frameAdapter
      DoomSubtopology.doom
      DoomSubtopology.doomBufferManager
      DoomSubtopology.frameDownsampler
    }

    # Only the DOOM engine and frame adapter telemetry cross the hub: DoomFlight packetizes it with the merged
    # packet list (tools/merge_packets.py)
    telemetry connections instance hub {
      DoomSubtopology.doom
      frameAdapter
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
      rateGroupDoom.RateGroupMemberOut[0] -> DoomSubtopology.doom.schedIn

      rateGroupDriver.CycleOut[Ports_RateGroups.rateGroup1Hz] -> rateGroup1Hz.CycleIn
      rateGroup1Hz.RateGroupMemberOut[0] -> DoomSubtopology.doomBufferManager.schedIn
    }

    connections HubCommands {
      hub.cmdDispOut -> cmdDisp.seqCmdBuff
      cmdDisp.seqCmdStatus -> hub.cmdRespIn
    }

    connections FramePipeline {
      DoomSubtopology.doom.frameOut               -> DoomSubtopology.frameDownsampler.frameIn
      DoomSubtopology.doom.paletteOut             -> DoomSubtopology.frameDownsampler.paletteIn
      DoomSubtopology.frameDownsampler.frameOut   -> frameAdapter.frameIn
      DoomSubtopology.frameDownsampler.paletteOut -> frameAdapter.paletteIn
    }

    connections HubFrames {
      # Packed frames and palettes to DoomFlight. The hub copies each buffer and returns it before the call ends.
      frameAdapter.packedOut             -> hub.bufferIn[HUB_FRAME_PORT]
      hub.bufferInReturn[HUB_FRAME_PORT] -> frameAdapter.packedOutReturn

      # DoomFlight's echo of each frame and palette, checked and counted by frameAdapter
      hub.bufferOut[HUB_FRAME_PORT]      -> frameAdapter.packedIn
      frameAdapter.packedInReturn        -> hub.bufferOutReturn[HUB_FRAME_PORT]
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
