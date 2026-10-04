module Components {

  @ Number of outputs of each FrameRepeater output port array
  constant FRAME_REPEATER_OUTPUT_PORTS = 2

  @ Repeats DOOM frames and palettes to several consumers on their native port types.
  @
  @ Doom.RawFrame lends the caller's pixel storage for the duration of the call, so the frame is fanned out by
  @ invoking every connected frameOut port in index order with the same pixels: nothing is copied or allocated and
  @ consumers must treat the pixels as read-only. Palettes are passed by value.
  @
  @ The handlers keep no shared state between frameIn and paletteIn, so each may be driven from its own thread;
  @ concurrent calls on the same input port are not supported.
  passive component FrameRepeater {

    # ----------------------------------------------------------------------
    # Ports
    # ----------------------------------------------------------------------

    @ Frame to repeat
    sync input port frameIn: Doom.RawFrame

    @ Palette to repeat
    sync input port paletteIn: Doom.PaletteSend

    @ Repeated frames: every connected port is invoked, in index order, with the pixels offered on frameIn
    output port frameOut: [FRAME_REPEATER_OUTPUT_PORTS] Doom.RawFrame

    @ Repeated palettes: every connected port is invoked, in index order
    output port paletteOut: [FRAME_REPEATER_OUTPUT_PORTS] Doom.PaletteSend

    # ----------------------------------------------------------------------
    # Telemetry
    # ----------------------------------------------------------------------

    @ Frames received on frameIn and repeated
    telemetry FramesRepeated: U32

    @ Palettes received on paletteIn and repeated
    telemetry PalettesRepeated: U32

    # ----------------------------------------------------------------------
    # Standard ports
    # ----------------------------------------------------------------------

    @ Telemetry port
    telemetry port tlmOut

    @ Time get port
    time get port timeCaller

  }

}
