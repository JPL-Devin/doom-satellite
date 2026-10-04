module Components {

    @ Content of a packed buffer, carried in its first byte
    enum FrameBufferKind : U8 {
        FRAME = 0    @< U32 frameNumber, U16 width, U16 height, then width * height pixels
        PALETTE = 1  @< Serialized Doom.Palette
    }

    @ Outcome of packing or unpacking a frame or palette
    enum FrameBufferStatus {
        OK = 0             @< Converted
        NULL_BUFFER = 1    @< Buffer has no data pointer
        SHORT_BUFFER = 2   @< Buffer is smaller than its header or than width * height pixels
        BAD_KIND = 3       @< First byte is not a FrameBufferKind
        BAD_GEOMETRY = 4   @< Width or height is zero or exceeds the downsampled frame size
        SIZE_MISMATCH = 5  @< Buffer size does not match the frame geometry or the packed palette size
        BUFFER_IN_USE = 6  @< The previous packed buffer of this kind has not been returned
    } default OK

    @ Converts Doom.RawFrame and Doom.PaletteSend to and from Fw.Buffer so frames and palettes can share one
    @ GenericHub buffer port. Each packed buffer starts with a FrameBufferKind byte.
    passive component FrameBufferAdapter {

        # ----------------------------------------------------------------------
        # Pack: typed frames and palettes to buffers
        # ----------------------------------------------------------------------

        @ Frame to pack
        guarded input port frameIn: Doom.RawFrame

        @ Palette to pack
        guarded input port paletteIn: Doom.PaletteSend

        @ Packed frames and palettes; each buffer is component storage and must be returned on packedOutReturn
        output port packedOut: Fw.BufferSend

        @ Return of buffers sent on packedOut
        sync input port packedOutReturn: Fw.BufferSend

        # ----------------------------------------------------------------------
        # Unpack: buffers to typed frames and palettes
        # ----------------------------------------------------------------------

        @ Packed frame or palette to unpack
        guarded input port packedIn: Fw.BufferSend

        @ Return of every buffer received on packedIn
        output port packedInReturn: Fw.BufferSend

        @ Unpacked frame; the pixels reference the received buffer and are valid only during the call
        output port frameOut: Doom.RawFrame

        @ Unpacked palette
        output port paletteOut: Doom.PaletteSend

        # ----------------------------------------------------------------------
        # Events and telemetry
        # ----------------------------------------------------------------------

        @ A buffer received on packedIn was not unpacked because its kind could not be read
        event PackedRejected(
            reason: FrameBufferStatus @< Failed check
            $size: FwSizeType @< Buffer size in bytes
        ) severity warning low \
            format "Packed buffer rejected ({}): {} bytes" \
            throttle 5

        @ A frame was not packed or unpacked
        event FrameRejected(
            reason: FrameBufferStatus @< Failed check
            width: U16 @< Frame width (0 when the header was not read)
            height: U16 @< Frame height (0 when the header was not read)
            $size: FwSizeType @< Buffer size in bytes
        ) severity warning low \
            format "Frame rejected ({}): {}x{} in {} bytes" \
            throttle 5

        @ A palette was not packed or unpacked
        event PaletteRejected(
            reason: FrameBufferStatus @< Failed check
            $size: FwSizeType @< Buffer size in bytes
        ) severity warning low \
            format "Palette rejected ({}): {} bytes" \
            throttle 5

        @ Buffers received on packedIn that were neither a frame nor a palette
        telemetry PackedRejected: U32

        @ Frames packed onto packedOut
        telemetry FramesPacked: U32

        @ Frames unpacked onto frameOut
        telemetry FramesUnpacked: U32

        @ Frames rejected in either direction
        telemetry FramesRejected: U32

        @ Palettes packed onto packedOut
        telemetry PalettesPacked: U32

        @ Palettes unpacked onto paletteOut
        telemetry PalettesUnpacked: U32

        @ Palettes rejected in either direction
        telemetry PalettesRejected: U32

        time get port timeCaller
        import Fw.Event
        import Fw.Channel
    }
}
