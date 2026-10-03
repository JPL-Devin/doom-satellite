module Components {
    @ Receives telemetry serialized by Components.TlmSplitter (via Svc.GenericHub serialOut) and re-emits it as typed
    @ telemetry, so consumers on this node see the stream as coming from the control node
    passive component TlmEchoReceiver {
        @ Serialized telemetry: channel id, time tag, then the telemetry buffer
        sync input port serialIn: serial

        @ Typed telemetry recovered from serialIn
        output port tlmOut: Fw.Tlm

        @ Periodic status reporting
        sync input port schedIn: Svc.Sched

        @ Echoed telemetry received since the last report
        event EchoStatus(
                          received: U32 @< Telemetry samples received since the last report
                          errors: U32 @< Samples that failed to deserialize since the last report
                          total: U32 @< Telemetry samples received since startup
                        ) \
          severity activity low \
          format "Echoed telemetry: {} received, {} errors, {} total"

        import Fw.Event
        time get port timeCaller
    }
}
