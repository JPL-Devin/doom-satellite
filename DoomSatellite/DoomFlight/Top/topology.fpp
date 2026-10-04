module DoomFlight {

  # ----------------------------------------------------------------------
  # Symbolic constants for port numbers
  # ----------------------------------------------------------------------

  enum Ports_RateGroups {
    rateGroup10Hz
    rateGroup1Hz
  }

  @ Hub buffer port carrying packed downsampled frames and palettes from DoomCoprocessor, and their echo back
  constant HUB_FRAME_PORT = 0

  @ Repeater outputs
  enum Ports_FrameRepeater {
    ECHO   @< Echo to DoomCoprocessor
    LOCAL  @< Local frame telemetry
  }

  deployment topology DoomFlight {

  # ----------------------------------------------------------------------
  # Subtopology imports
  # ----------------------------------------------------------------------
    import CdhCore.Subtopology
    import ComCcsds.Subtopology

  # ----------------------------------------------------------------------
  # Instances used in the topology
  # ----------------------------------------------------------------------
    instance chronoTime
    instance rateGroup10Hz
    instance rateGroup1Hz
    instance rateGroupDriver
    instance timer
    instance comDriver
    instance nullPrmDb
    instance touchReset
    instance cmdSplitter
    instance frameRepeater
    instance frameAdapter
    instance frameTlmProcessor
    instance hub
    instance hubComDriver
    instance hubByteStreamAdapter
    instance hubBufferManager

  # ----------------------------------------------------------------------
  # Pattern graph specifiers
  # ----------------------------------------------------------------------

    command connections instance CdhCore.cmdDisp
    event connections instance CdhCore.events
    text event connections instance CdhCore.textLogger
    health connections instance CdhCore.$health
    time connections instance chronoTime
    telemetry connections instance CdhCore.tlmSend
    param connections instance nullPrmDb

  # ----------------------------------------------------------------------
  # Telemetry packets
  # ----------------------------------------------------------------------

  include "DoomFlightPackets.fppi"

  # ----------------------------------------------------------------------
  # Direct graph specifiers
  # ----------------------------------------------------------------------

    connections ComCcsds_CdhCore {
      # Core events and telemetry to communication queue
      CdhCore.events.PktSend -> ComCcsds.comQueue.comPacketQueueIn[ComCcsds.Ports_ComPacketQueue.EVENTS]
      CdhCore.tlmSend.PktSend -> ComCcsds.comQueue.comPacketQueueIn[ComCcsds.Ports_ComPacketQueue.TELEMETRY]

      # Router to command splitter: local opcodes to the command dispatcher, remote opcodes to the hub
      ComCcsds.fprimeRouter.commandOut -> cmdSplitter.CmdBuff
      cmdSplitter.LocalCmd[0]          -> CdhCore.cmdDisp.seqCmdBuff
      CdhCore.cmdDisp.seqCmdStatus     -> cmdSplitter.seqCmdStatus
      cmdSplitter.forwardSeqCmdStatus  -> ComCcsds.fprimeRouter.cmdResponseIn
    }

    connections Communications {
      # ComDriver buffer allocations
      comDriver.allocate      -> ComCcsds.commsBufferManager.bufferGetCallee
      comDriver.deallocate    -> ComCcsds.commsBufferManager.bufferSendIn

      # ComDriver <-> ComStub (Uplink)
      comDriver.$recv                     -> ComCcsds.comStub.drvReceiveIn
      ComCcsds.comStub.drvReceiveReturnOut -> comDriver.recvReturnIn

      # ComStub <-> ComDriver (Downlink)
      ComCcsds.comStub.drvSendOut      -> comDriver.$send
      comDriver.ready         -> ComCcsds.comStub.drvConnected
    }

    connections HubRemote {
      # Commands for the remote deployment and their responses
      cmdSplitter.RemoteCmd -> hub.cmdDispIn
      hub.cmdRespOut        -> ComCcsds.fprimeRouter.cmdResponseIn

      # Remote events join the local downlink
      hub.eventOut -> CdhCore.events.LogRecv
    }

    connections HubTelemetry {
      # Remote (DOOM) telemetry is packetized for the local downlink using the merged packet list
      # (tools/merge_packets.py)
      hub.tlmOut -> CdhCore.tlmSend.TlmRecv
    }

    connections HubFrames {
      # The repeater copies each packed frame or palette from the hub, returns the original to the hub, and sends one
      # copy back to DoomCoprocessor and one to frameAdapter. The echo copy is relayed by frameAdapter because the hub
      # returns each bufferIn buffer to the instance that sent it, and the copy must go back to hubBufferManager.
      hub.bufferOut[HUB_FRAME_PORT]                    -> frameRepeater.portIn
      frameRepeater.deallocate                         -> hub.bufferOutReturn[HUB_FRAME_PORT]
      frameRepeater.allocate                           -> hubBufferManager.bufferGetCallee
      frameRepeater.portOut[Ports_FrameRepeater.ECHO]  -> frameAdapter.echoIn
      frameAdapter.echoOut                             -> hub.bufferIn[HUB_FRAME_PORT]
      hub.bufferInReturn[HUB_FRAME_PORT]               -> frameAdapter.echoOutReturn
      frameAdapter.echoReturn                          -> hubBufferManager.bufferSendIn
      frameRepeater.portOut[Ports_FrameRepeater.LOCAL] -> frameAdapter.packedIn
      frameAdapter.packedInReturn                      -> hubBufferManager.bufferSendIn

      # Unpacked frames and palettes to frame telemetry
      frameAdapter.frameOut   -> frameTlmProcessor.frameIn
      frameAdapter.paletteOut -> frameTlmProcessor.paletteIn
    }

    connections Hub {
      # Hub -> adapter -> UDP (send)
      hub.toBufferDriver                    -> hubByteStreamAdapter.bufferIn
      hubByteStreamAdapter.bufferInReturn   -> hub.toBufferDriverReturn
      hubByteStreamAdapter.toByteStreamDriver -> hubComDriver.$send

      # UDP -> adapter -> hub (receive)
      hubComDriver.$recv                                -> hubByteStreamAdapter.fromByteStreamDriver
      hubByteStreamAdapter.fromByteStreamDriverReturn   -> hubComDriver.recvReturnIn
      hubByteStreamAdapter.bufferOut                    -> hub.fromBufferDriver
      hub.fromBufferDriverReturn                        -> hubByteStreamAdapter.bufferOutReturn
      hubComDriver.ready                                -> hubByteStreamAdapter.byteStreamDriverReady

      # Shared buffer allocation
      hub.allocate            -> hubBufferManager.bufferGetCallee
      hub.deallocate          -> hubBufferManager.bufferSendIn
      hubComDriver.allocate   -> hubBufferManager.bufferGetCallee
      hubComDriver.deallocate -> hubBufferManager.bufferSendIn
    }

    connections RateGroups {
      # timer to drive rate group
      timer.CycleOut -> rateGroupDriver.CycleIn

      # High rate (10Hz) rate group
      rateGroupDriver.CycleOut[Ports_RateGroups.rateGroup10Hz] -> rateGroup10Hz.CycleIn
      rateGroup10Hz.RateGroupMemberOut[0] -> comDriver.schedIn

      # Slow rate (1Hz) rate group
      rateGroupDriver.CycleOut[Ports_RateGroups.rateGroup1Hz] -> rateGroup1Hz.CycleIn
      rateGroup1Hz.RateGroupMemberOut[0] -> ComCcsds.comQueue.run
      rateGroup1Hz.RateGroupMemberOut[1] -> CdhCore.$health.Run
      rateGroup1Hz.RateGroupMemberOut[2] -> ComCcsds.commsBufferManager.schedIn
      rateGroup1Hz.RateGroupMemberOut[3] -> CdhCore.tlmSend.Run
      rateGroup1Hz.RateGroupMemberOut[4] -> ComCcsds.aggregator.timeout
      rateGroup1Hz.RateGroupMemberOut[5] -> CdhCore.Subtopology.eventsRun
      rateGroup1Hz.RateGroupMemberOut[6] -> hubBufferManager.schedIn
    }

    connections DoomFlight {

    }

  }

}
