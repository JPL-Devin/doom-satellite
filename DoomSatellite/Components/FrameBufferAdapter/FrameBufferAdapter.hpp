// ======================================================================
// \title  FrameBufferAdapter.hpp
// \brief  hpp file for FrameBufferAdapter component implementation class
// ======================================================================

#ifndef Components_FrameBufferAdapter_HPP
#define Components_FrameBufferAdapter_HPP

#include "Doom/DoomConfig/FppConstantsAc.hpp"
#include "DoomSatellite/Components/FrameBufferAdapter/FrameBufferAdapterComponentAc.hpp"
#include "DoomSatellite/Components/FrameBufferAdapter/FrameBufferKindEnumAc.hpp"

namespace Components {

class FrameBufferAdapter final : public FrameBufferAdapterComponentBase {
  public:
    //! Leading kind byte of every packed buffer
    static constexpr FwSizeType KIND_SIZE = FrameBufferKind::SERIALIZED_SIZE;
    //! Packed frame header: kind, U32 frame number, U16 width, U16 height
    static constexpr FwSizeType FRAME_HEADER_SIZE = KIND_SIZE + sizeof(U32) + sizeof(U16) + sizeof(U16);
    //! Largest frame accepted: the downsampled frame
    static constexpr FwSizeType MAX_FRAME_PIXELS =
        static_cast<FwSizeType>(Doom::DOWNSAMPLED_WIDTH) * static_cast<FwSizeType>(Doom::DOWNSAMPLED_HEIGHT);
    //! Largest packed frame
    static constexpr FwSizeType MAX_PACKED_FRAME_SIZE = FRAME_HEADER_SIZE + MAX_FRAME_PIXELS;
    //! Packed palette size: kind and the serialized palette
    static constexpr FwSizeType PACKED_PALETTE_SIZE = KIND_SIZE + Doom::Palette::SERIALIZED_SIZE;

    //! Construct FrameBufferAdapter object
    explicit FrameBufferAdapter(const char* const compName  //!< The component name
    );

    //! Destroy FrameBufferAdapter object
    ~FrameBufferAdapter();

  private:
    void frameIn_handler(FwIndexType portNum, U32 frameNumber, U16 width, U16 height, Fw::Buffer& pixels) override;

    void paletteIn_handler(FwIndexType portNum, const Doom::Palette& palette) override;

    void packedOutReturn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) override;

    void packedIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) override;

    //! Check frame geometry against the downsampled frame size
    static FrameBufferStatus checkGeometry(U16 width, U16 height);

    //! Check a frame offered on frameIn before packing
    FrameBufferStatus checkFrameIn(U16 width, U16 height, const Fw::Buffer& pixels) const;

    //! Validate a packed frame and emit it on frameOut when connected
    FrameBufferStatus unpackFrame(const Fw::Buffer& fwBuffer, U16& width, U16& height);

    //! Validate a packed palette and emit it on paletteOut when connected
    FrameBufferStatus unpackPalette(const Fw::Buffer& fwBuffer);

    //! Unpack a buffer whose kind is a frame, counting and reporting the outcome
    void handlePackedFrame(const Fw::Buffer& fwBuffer);

    //! Unpack a buffer whose kind is a palette, counting and reporting the outcome
    void handlePackedPalette(const Fw::Buffer& fwBuffer);

    //! Count and report a buffer whose kind could not be read
    void rejectPacked(FrameBufferStatus reason, FwSizeType size);

    //! Count and report a rejected frame
    void rejectFrame(FrameBufferStatus reason, U16 width, U16 height, FwSizeType size);

    //! Count and report a rejected palette
    void rejectPalette(FrameBufferStatus reason, FwSizeType size);

    U8 m_frameStorage[MAX_PACKED_FRAME_SIZE];  //!< Packed frame storage lent on packedOut
    U8 m_paletteStorage[PACKED_PALETTE_SIZE];  //!< Packed palette storage lent on packedOut
    bool m_frameLent;                          //!< m_frameStorage is out on packedOut
    bool m_paletteLent;                        //!< m_paletteStorage is out on packedOut
    U32 m_packedRejected;                      //!< Buffers of unreadable kind
    U32 m_framesPacked;                        //!< Frames packed
    U32 m_framesUnpacked;                      //!< Frames unpacked
    U32 m_framesRejected;                      //!< Frames rejected
    U32 m_palettesPacked;                      //!< Palettes packed
    U32 m_palettesUnpacked;                    //!< Palettes unpacked
    U32 m_palettesRejected;                    //!< Palettes rejected
};

}  // namespace Components

#endif
