module Components {
    @ Duplicates a telemetry stream: one copy continues as typed telemetry (e.g. into the local TlmPacketizer) and a
    @ second copy is serialized out of a serial port (e.g. into Svc.GenericHub serialIn to echo it to a remote node)
    passive component TlmSplitter {
        @ Telemetry stream to duplicate
        sync input port tlmIn: Fw.Tlm

        @ Typed copy of the telemetry stream
        output port tlmOut: Fw.Tlm

        @ Serialized copy of the telemetry stream: channel id, time tag, then the telemetry buffer
        output port echoOut: serial
    }
}
