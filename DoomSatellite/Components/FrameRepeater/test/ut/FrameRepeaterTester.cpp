// ======================================================================
// \title  FrameRepeaterTester.cpp
// \brief  cpp file for FrameRepeater component test harness implementation class
// ======================================================================

#include "FrameRepeaterTester.hpp"

#include "Doom/DoomConfig/FppConstantsAc.hpp"

namespace Components {

constexpr FwSizeType FrameRepeaterTester::PIXELS;

FrameRepeaterTester::FrameRepeaterTester()
    : FrameRepeaterGTestBase("FrameRepeaterTester", FrameRepeaterTester::MAX_HISTORY_SIZE),
      component("FrameRepeater"),
      m_pixels{},
      m_frameOrder{},
      m_paletteOrder{},
      m_frameCalls(0),
      m_paletteCalls(0) {
    this->initComponents();
    this->connectPorts();
    for (FwSizeType i = 0; i < PIXELS; i++) {
        this->m_pixels[i] = static_cast<U8>(i * 3U);
    }
}

FrameRepeaterTester::~FrameRepeaterTester() {}

// ----------------------------------------------------------------------
// Handlers
// ----------------------------------------------------------------------

void FrameRepeaterTester::from_frameOut_handler(FwIndexType portNum,
                                                U32 frameNumber,
                                                U16 width,
                                                U16 height,
                                                Fw::Buffer& pixels) {
    this->pushFromPortEntry_frameOut(frameNumber, width, height, pixels);
    ASSERT_LT(this->m_frameCalls, FW_NUM_ARRAY_ELEMENTS(this->m_frameOrder));
    this->m_frameOrder[this->m_frameCalls++] = portNum;
    ASSERT_EQ(pixels.getData(), this->m_pixels);
    ASSERT_EQ(pixels.getSize(), PIXELS);
    // A consumer shrinking its view must not affect the next consumer
    pixels.setSize(0);
}

void FrameRepeaterTester::from_paletteOut_handler(FwIndexType portNum, const Doom::Palette& palette) {
    this->pushFromPortEntry_paletteOut(palette);
    ASSERT_LT(this->m_paletteCalls, FW_NUM_ARRAY_ELEMENTS(this->m_paletteOrder));
    this->m_paletteOrder[this->m_paletteCalls++] = portNum;
}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------

void FrameRepeaterTester::testRepeatsFrames() {
    Fw::Buffer pixels(this->m_pixels, PIXELS);
    this->invoke_to_frameIn(0, 7, 8, 8, pixels);
    ASSERT_from_frameOut_SIZE(FrameRepeater::NUM_FRAMEOUT_OUTPUT_PORTS);
    ASSERT_EQ(this->m_frameCalls, static_cast<FwSizeType>(FrameRepeater::NUM_FRAMEOUT_OUTPUT_PORTS));
    for (FwIndexType i = 0; i < FrameRepeater::NUM_FRAMEOUT_OUTPUT_PORTS; i++) {
        ASSERT_EQ(this->m_frameOrder[i], i);
        const FromPortEntry_frameOut& entry = this->fromPortHistory_frameOut->at(static_cast<FwSizeType>(i));
        ASSERT_EQ(entry.frameNumber, 7U);
        ASSERT_EQ(entry.width, 8U);
        ASSERT_EQ(entry.height, 8U);
    }
    // The caller's buffer is untouched by consumers redirecting their own views
    ASSERT_EQ(pixels.getData(), this->m_pixels);
    ASSERT_EQ(pixels.getSize(), PIXELS);
    ASSERT_TLM_SIZE(1);
    ASSERT_TLM_FramesRepeated_SIZE(1);
    ASSERT_TLM_FramesRepeated(0, 1U);
    ASSERT_from_paletteOut_SIZE(0);

    this->m_frameCalls = 0;
    this->invoke_to_frameIn(0, 8, 8, 8, pixels);
    ASSERT_TLM_FramesRepeated(1, 2U);
}

void FrameRepeaterTester::testRepeatsPalettes() {
    Doom::Palette palette;
    palette.set_generation(3);
    for (FwSizeType i = 0; i < Doom::PALETTE_BYTES; i++) {
        palette.get_rgb()[i] = static_cast<U8>(i);
    }
    this->invoke_to_paletteIn(0, palette);
    ASSERT_from_paletteOut_SIZE(FrameRepeater::NUM_PALETTEOUT_OUTPUT_PORTS);
    for (FwIndexType i = 0; i < FrameRepeater::NUM_PALETTEOUT_OUTPUT_PORTS; i++) {
        ASSERT_EQ(this->m_paletteOrder[i], i);
        ASSERT_from_paletteOut(static_cast<FwSizeType>(i), palette);
    }
    ASSERT_TLM_SIZE(1);
    ASSERT_TLM_PalettesRepeated(0, 1U);
    ASSERT_from_frameOut_SIZE(0);
}

void FrameRepeaterTester::testSkipsUnconnectedOutputs() {
    // Only the last output of each array is connected
    FrameRepeater partial("PartialFrameRepeater");
    partial.init(TEST_INSTANCE_ID);
    const FwIndexType last = FrameRepeater::NUM_FRAMEOUT_OUTPUT_PORTS - 1;
    partial.set_frameOut_OutputPort(last, this->get_from_frameOut(last));
    partial.set_paletteOut_OutputPort(last, this->get_from_paletteOut(last));
    partial.set_tlmOut_OutputPort(0, this->get_from_tlmOut(0));
    partial.set_timeCaller_OutputPort(0, this->get_from_timeCaller(0));

    Fw::Buffer pixels(this->m_pixels, PIXELS);
    partial.get_frameIn_InputPort(0)->invoke(1, 8, 8, pixels);
    ASSERT_from_frameOut_SIZE(1);
    ASSERT_EQ(this->m_frameOrder[0], last);

    Doom::Palette palette;
    partial.get_paletteIn_InputPort(0)->invoke(palette);
    ASSERT_from_paletteOut_SIZE(1);
    ASSERT_EQ(this->m_paletteOrder[0], last);

    ASSERT_TLM_FramesRepeated(0, 1U);
    ASSERT_TLM_PalettesRepeated(0, 1U);
}

}  // namespace Components
