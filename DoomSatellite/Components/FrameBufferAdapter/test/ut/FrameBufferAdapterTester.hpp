// ======================================================================
// \title  FrameBufferAdapterTester.hpp
// \brief  hpp file for FrameBufferAdapter component test harness implementation class
// ======================================================================

#ifndef Components_FrameBufferAdapterTester_HPP
#define Components_FrameBufferAdapterTester_HPP

#include "DoomSatellite/Components/FrameBufferAdapter/FrameBufferAdapter.hpp"
#include "DoomSatellite/Components/FrameBufferAdapter/FrameBufferAdapterGTestBase.hpp"

namespace Components {

class FrameBufferAdapterTester final : public FrameBufferAdapterGTestBase {
  public:
    static constexpr FwSizeType MAX_HISTORY_SIZE = 16;
    static constexpr FwEnumStoreType TEST_INSTANCE_ID = 0;
    static constexpr U16 WIDTH = static_cast<U16>(Doom::DOWNSAMPLED_WIDTH);
    static constexpr U16 HEIGHT = static_cast<U16>(Doom::DOWNSAMPLED_HEIGHT);
    static constexpr FwSizeType PIXELS = FrameBufferAdapter::MAX_FRAME_PIXELS;

    FrameBufferAdapterTester();
    ~FrameBufferAdapterTester();

    void testPacksFrame();
    void testPacksPalette();
    void testRejectsInvalidFrameIn();
    void testRejectsWhileLent();
    void testRoundTripsFrame();
    void testRoundTripsPalette();
    void testRejectsInvalidPacked();
    void testRejectsInvalidPackedFrame();
    void testRejectsInvalidPackedPalette();

  private:
    void connectPorts();
    void initComponents();

    //! Copy each packed buffer and return it unless m_holdPacked is set
    void from_packedOut_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) override;

    //! Copy the unpacked pixels, which are valid only during the call
    void from_frameOut_handler(FwIndexType portNum,
                               U32 frameNumber,
                               U16 width,
                               U16 height,
                               Fw::Buffer& pixels) override;

    //! Fill m_pixels with a position-dependent pattern
    void fillPixels(U32 seed);

    //! Palette with a position-dependent pattern
    static Doom::Palette makePalette(U32 generation);

    //! Pack a frame and a palette into m_packed / m_packedSize via frameIn and paletteIn
    void packFrame(U32 frameNumber, U16 width, U16 height);
    void packPalette(const Doom::Palette& palette);

    //! Send m_packed[0..size) on packedIn and check it is returned unchanged
    void sendPacked(FwSizeType size);

    FrameBufferAdapter component;

    U8 m_pixels[PIXELS];                                          //!< Frame offered on frameIn
    U8 m_packed[FrameBufferAdapter::MAX_PACKED_FRAME_SIZE + 16];  //!< Last buffer seen on packedOut
    FwSizeType m_packedSize;                                      //!< Size of the last packedOut buffer
    U8 m_unpacked[PIXELS];                                        //!< Pixels seen on frameOut
    bool m_holdPacked;                                            //!< Do not return packedOut buffers
};

}  // namespace Components

#endif
