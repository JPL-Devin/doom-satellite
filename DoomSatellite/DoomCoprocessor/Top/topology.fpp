module DoomCoprocessor {

  deployment topology DoomCoprocessor {

  # ----------------------------------------------------------------------
  # Instances used in the topology
  # ----------------------------------------------------------------------
    instance cmdDisp
    instance chronoTime
    instance hub
    instance hubComDriver
    instance hubByteStreamAdapter
    instance hubBufferManager

  # ----------------------------------------------------------------------
  # Pattern graph specifiers
  # ----------------------------------------------------------------------

    command connections instance cmdDisp

    # Hub transport components are excluded: their events would re-enter the hub that emitted them
    event connections instance hub {
      cmdDisp
    }

    # Telemetry is not sent across the hub: DoomFlight's TlmPacketizer cannot track remote channels
    time connections instance chronoTime

  # ----------------------------------------------------------------------
  # Direct graph specifiers
  # ----------------------------------------------------------------------

    connections HubCommands {
      hub.cmdDispOut           -> cmdDisp.seqCmdBuff
      cmdDisp.seqCmdStatus     -> hub.cmdRespIn
    }

    connections Hub {
      hub.toBufferDriver                      -> hubByteStreamAdapter.bufferIn
      hubByteStreamAdapter.bufferInReturn     -> hub.toBufferDriverReturn
      hubByteStreamAdapter.toByteStreamDriver -> hubComDriver.$send

      hubComDriver.$recv                              -> hubByteStreamAdapter.fromByteStreamDriver
      hubByteStreamAdapter.fromByteStreamDriverReturn -> hubComDriver.recvReturnIn
      hubByteStreamAdapter.bufferOut                  -> hub.fromBufferDriver
      hub.fromBufferDriverReturn                      -> hubByteStreamAdapter.bufferOutReturn
      hubComDriver.ready                              -> hubByteStreamAdapter.byteStreamDriverReady

      hub.allocate            -> hubBufferManager.bufferGetCallee
      hub.deallocate          -> hubBufferManager.bufferSendIn
      hubComDriver.allocate   -> hubBufferManager.bufferGetCallee
      hubComDriver.deallocate -> hubBufferManager.bufferSendIn
    }

    connections DoomCoprocessor {
      # Add here connections to user-defined components
    }

  }

}
