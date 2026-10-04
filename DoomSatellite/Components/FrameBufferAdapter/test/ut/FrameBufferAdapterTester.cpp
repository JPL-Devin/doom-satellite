// ======================================================================
// \title  FrameBufferAdapterTester.cpp
// \brief  cpp file for FrameBufferAdapter component test harness implementation class
// ======================================================================

#include "FrameBufferAdapterTester.hpp"

#include <cstring>

namespace Components {

namespace {
constexpr FwSizeType PALETTE_SIZE = FrameBufferAdapter::PACKED_PALETTE_SIZE;
constexpr FwSizeType HEADER_SIZE = FrameBufferAdapter::FRAME_HEADER_SIZE;
}  // namespace

constexpr U16 FrameBufferAdapterTester::WIDTH;
constexpr U16 FrameBufferAdapterTester::HEIGHT;
constexpr FwSizeType FrameBufferAdapterTester::PIXELS;

FrameBufferAdapterTester::FrameBufferAdapterTester()
    : FrameBufferAdapterGTestBase("FrameBufferAdapterTester", FrameBufferAdapterTester::MAX_HISTORY_SIZE),
      component("FrameBufferAdapter"),
      m_pixels{},
      m_packed{},
      m_packedSize(0),
      m_unpacked{},
      m_holdPacked(false) {
    this->initComponents();
    this->connectPorts();
}

FrameBufferAdapterTester::~FrameBufferAdapterTester() {}

// ----------------------------------------------------------------------
// Handlers and helpers
// ----------------------------------------------------------------------

void FrameBufferAdapterTester::from_packedOut_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    this->pushFromPortEntry_packedOut(fwBuffer);
    ASSERT_NE(fwBuffer.getData(), nullptr);
    ASSERT_LE(fwBuffer.getSize(), sizeof(this->m_packed));
    (void)::memcpy(this->m_packed, fwBuffer.getData(), static_cast<size_t>(fwBuffer.getSize()));
    this->m_packedSize = fwBuffer.getSize();
    if (!this->m_holdPacked) {
        this->invoke_to_packedOutReturn(0, fwBuffer);
    }
}

void FrameBufferAdapterTester::from_frameOut_handler(FwIndexType portNum,
                                                     U32 frameNumber,
                                                     U16 width,
                                                     U16 height,
                                                     Fw::Buffer& pixels) {
    this->pushFromPortEntry_frameOut(frameNumber, width, height, pixels);
    ASSERT_LE(pixels.getSize(), sizeof(this->m_unpacked));
    (void)::memcpy(this->m_unpacked, pixels.getData(), static_cast<size_t>(pixels.getSize()));
}

void FrameBufferAdapterTester::fillPixels(U32 seed) {
    for (FwSizeType i = 0; i < PIXELS; i++) {
        this->m_pixels[i] = static_cast<U8>((i + seed) % 251U);
    }
}

Doom::Palette FrameBufferAdapterTester::makePalette(U32 generation) {
    Doom::Palette palette;
    palette.set_generation(generation);
    for (FwSizeType i = 0; i < Doom::PALETTE_BYTES; i++) {
        palette.get_rgb()[i] = static_cast<U8>((i * 7U + generation) % 256U);
    }
    return palette;
}

void FrameBufferAdapterTester::packFrame(U32 frameNumber, U16 width, U16 height) {
    Fw::Buffer pixels(this->m_pixels, static_cast<FwSizeType>(width) * static_cast<FwSizeType>(height));
    this->invoke_to_frameIn(0, frameNumber, width, height, pixels);
}

void FrameBufferAdapterTester::packPalette(const Doom::Palette& palette) {
    this->invoke_to_paletteIn(0, palette);
}

void FrameBufferAdapterTester::sendPacked(FwSizeType size) {
    const FwSizeType returnsBefore = this->fromPortHistory_packedInReturn->size();
    Fw::Buffer packed(this->m_packed, size);
    this->invoke_to_packedIn(0, packed);
    ASSERT_EQ(this->fromPortHistory_packedInReturn->size(), returnsBefore + 1);
    const Fw::Buffer& returned = this->fromPortHistory_packedInReturn->at(returnsBefore).fwBuffer;
    ASSERT_EQ(returned.getData(), this->m_packed);
    ASSERT_EQ(returned.getSize(), size);
}

// ----------------------------------------------------------------------
// Pack
// ----------------------------------------------------------------------

void FrameBufferAdapterTester::testPacksFrame() {
    this->fillPixels(3);
    this->packFrame(0x01020304U, WIDTH, HEIGHT);

    ASSERT_from_packedOut_SIZE(1);
    ASSERT_EQ(this->m_packedSize, HEADER_SIZE + PIXELS);
    // Big-endian header: kind, frame number, width, height
    ASSERT_EQ(this->m_packed[0], FrameBufferKind::FRAME);
    ASSERT_EQ(this->m_packed[1], 0x01);
    ASSERT_EQ(this->m_packed[2], 0x02);
    ASSERT_EQ(this->m_packed[3], 0x03);
    ASSERT_EQ(this->m_packed[4], 0x04);
    ASSERT_EQ(this->m_packed[5], static_cast<U8>(WIDTH >> 8));
    ASSERT_EQ(this->m_packed[6], static_cast<U8>(WIDTH & 0xFFU));
    ASSERT_EQ(this->m_packed[7], static_cast<U8>(HEIGHT >> 8));
    ASSERT_EQ(this->m_packed[8], static_cast<U8>(HEIGHT & 0xFFU));
    ASSERT_EQ(::memcmp(&this->m_packed[HEADER_SIZE], this->m_pixels, PIXELS), 0);

    ASSERT_EVENTS_SIZE(0);
    ASSERT_TLM_FramesPacked_SIZE(1);
    ASSERT_TLM_FramesPacked(0, 1U);

    // The returned storage is reused for the next frame
    this->fillPixels(9);
    this->packFrame(2U, WIDTH, HEIGHT);
    ASSERT_from_packedOut_SIZE(2);
    ASSERT_EQ(::memcmp(&this->m_packed[HEADER_SIZE], this->m_pixels, PIXELS), 0);
    ASSERT_TLM_FramesPacked(1, 2U);
}

void FrameBufferAdapterTester::testPacksPalette() {
    const Doom::Palette palette = makePalette(7U);
    this->packPalette(palette);

    ASSERT_from_packedOut_SIZE(1);
    ASSERT_EQ(this->m_packedSize, PALETTE_SIZE);
    ASSERT_EQ(this->m_packed[0], FrameBufferKind::PALETTE);
    // Generation (big-endian U32) then the RGB bytes
    ASSERT_EQ(this->m_packed[4], 7U);
    ASSERT_EQ(::memcmp(&this->m_packed[5], palette.get_rgb(), Doom::PALETTE_BYTES), 0);
    ASSERT_EVENTS_SIZE(0);
    ASSERT_TLM_PalettesPacked(0, 1U);
}

void FrameBufferAdapterTester::testRejectsInvalidFrameIn() {
    this->fillPixels(0);
    Fw::Buffer pixels(this->m_pixels, PIXELS);

    this->invoke_to_frameIn(0, 1U, 0, HEIGHT, pixels);
    this->invoke_to_frameIn(0, 1U, WIDTH, 0, pixels);
    this->invoke_to_frameIn(0, 1U, static_cast<U16>(WIDTH + 1), HEIGHT, pixels);
    this->invoke_to_frameIn(0, 1U, WIDTH, static_cast<U16>(HEIGHT + 1), pixels);
    Fw::Buffer nullPixels(nullptr, PIXELS);
    this->invoke_to_frameIn(0, 1U, WIDTH, HEIGHT, nullPixels);
    Fw::Buffer shortPixels(this->m_pixels, PIXELS - 1);
    this->invoke_to_frameIn(0, 1U, WIDTH, HEIGHT, shortPixels);

    ASSERT_from_packedOut_SIZE(0);
    ASSERT_EVENTS_FrameRejected_SIZE(5);  // Throttled after five
    ASSERT_EVENTS_FrameRejected(0, FrameBufferStatus::BAD_GEOMETRY, 0, HEIGHT, PIXELS);
    ASSERT_EVENTS_FrameRejected(1, FrameBufferStatus::BAD_GEOMETRY, WIDTH, 0, PIXELS);
    ASSERT_EVENTS_FrameRejected(2, FrameBufferStatus::BAD_GEOMETRY, WIDTH + 1, HEIGHT, PIXELS);
    ASSERT_EVENTS_FrameRejected(3, FrameBufferStatus::BAD_GEOMETRY, WIDTH, HEIGHT + 1, PIXELS);
    ASSERT_EVENTS_FrameRejected(4, FrameBufferStatus::NULL_BUFFER, WIDTH, HEIGHT, PIXELS);
    ASSERT_TLM_FramesRejected_SIZE(6);
    ASSERT_TLM_FramesRejected(5, 6U);
    ASSERT_TLM_FramesPacked_SIZE(0);

    // A valid frame clears the throttle, so the short buffer is reported on the next rejection
    this->packFrame(2U, WIDTH, HEIGHT);
    ASSERT_from_packedOut_SIZE(1);
    this->invoke_to_frameIn(0, 1U, WIDTH, HEIGHT, shortPixels);
    ASSERT_EVENTS_FrameRejected_SIZE(6);
    ASSERT_EVENTS_FrameRejected(5, FrameBufferStatus::SHORT_BUFFER, WIDTH, HEIGHT, PIXELS - 1);
}

void FrameBufferAdapterTester::testRejectsWhileLent() {
    this->m_holdPacked = true;
    const Doom::Palette palette = makePalette(1U);
    this->fillPixels(0);
    this->packFrame(1U, WIDTH, HEIGHT);
    this->packPalette(palette);
    ASSERT_from_packedOut_SIZE(2);
    const Fw::Buffer heldFrame = this->fromPortHistory_packedOut->at(0).fwBuffer;
    const Fw::Buffer heldPalette = this->fromPortHistory_packedOut->at(1).fwBuffer;

    // Storage still lent: both kinds are refused rather than overwritten
    this->packFrame(2U, WIDTH, HEIGHT);
    this->packPalette(palette);
    ASSERT_from_packedOut_SIZE(2);
    ASSERT_EVENTS_FrameRejected_SIZE(1);
    ASSERT_EVENTS_FrameRejected(0, FrameBufferStatus::BUFFER_IN_USE, WIDTH, HEIGHT, PIXELS);
    ASSERT_EVENTS_PaletteRejected_SIZE(1);
    ASSERT_EVENTS_PaletteRejected(0, FrameBufferStatus::BUFFER_IN_USE, PALETTE_SIZE);

    // Returning each buffer frees only its own storage
    Fw::Buffer frameReturn = heldFrame;
    this->invoke_to_packedOutReturn(0, frameReturn);
    this->packPalette(palette);
    ASSERT_from_packedOut_SIZE(2);
    this->packFrame(3U, WIDTH, HEIGHT);
    ASSERT_from_packedOut_SIZE(3);

    Fw::Buffer paletteReturn = heldPalette;
    this->invoke_to_packedOutReturn(0, paletteReturn);
    this->packPalette(palette);
    ASSERT_from_packedOut_SIZE(4);
}

// ----------------------------------------------------------------------
// Unpack
// ----------------------------------------------------------------------

void FrameBufferAdapterTester::testRoundTripsFrame() {
    this->fillPixels(42);
    this->packFrame(77U, WIDTH, HEIGHT);
    this->sendPacked(this->m_packedSize);

    ASSERT_from_frameOut_SIZE(1);
    ASSERT_from_frameOut(0, 77U, WIDTH, HEIGHT, this->fromPortHistory_frameOut->at(0).pixels);
    const Fw::Buffer& pixels = this->fromPortHistory_frameOut->at(0).pixels;
    ASSERT_EQ(pixels.getData(), &this->m_packed[HEADER_SIZE]);
    ASSERT_EQ(pixels.getSize(), PIXELS);
    ASSERT_EQ(::memcmp(this->m_unpacked, this->m_pixels, PIXELS), 0);
    ASSERT_from_paletteOut_SIZE(0);
    ASSERT_EVENTS_SIZE(0);
    ASSERT_TLM_FramesUnpacked(0, 1U);

    // Smaller frames within the downsampled size are accepted
    this->packFrame(78U, 4, 3);
    this->sendPacked(this->m_packedSize);
    ASSERT_from_frameOut_SIZE(2);
    ASSERT_EQ(this->fromPortHistory_frameOut->at(1).width, 4);
    ASSERT_EQ(this->fromPortHistory_frameOut->at(1).height, 3);
    ASSERT_EQ(::memcmp(this->m_unpacked, this->m_pixels, 12), 0);
}

void FrameBufferAdapterTester::testRoundTripsPalette() {
    const Doom::Palette palette = makePalette(12U);
    this->packPalette(palette);
    this->sendPacked(this->m_packedSize);

    ASSERT_from_paletteOut_SIZE(1);
    ASSERT_from_paletteOut(0, palette);
    ASSERT_from_frameOut_SIZE(0);
    ASSERT_EVENTS_SIZE(0);
    ASSERT_TLM_PalettesUnpacked(0, 1U);
}

void FrameBufferAdapterTester::testRejectsInvalidPacked() {
    Fw::Buffer nullBuffer(nullptr, 10);
    this->invoke_to_packedIn(0, nullBuffer);
    ASSERT_from_packedInReturn_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_packedInReturn->at(0).fwBuffer.getData(), nullptr);

    this->sendPacked(0);
    this->m_packed[0] = 2U;  // Not a FrameBufferKind
    this->sendPacked(PALETTE_SIZE);

    ASSERT_from_frameOut_SIZE(0);
    ASSERT_from_paletteOut_SIZE(0);
    ASSERT_EVENTS_PackedRejected_SIZE(3);
    ASSERT_EVENTS_PackedRejected(0, FrameBufferStatus::NULL_BUFFER, 10);
    ASSERT_EVENTS_PackedRejected(1, FrameBufferStatus::SHORT_BUFFER, 0);
    ASSERT_EVENTS_PackedRejected(2, FrameBufferStatus::BAD_KIND, PALETTE_SIZE);
    ASSERT_TLM_PackedRejected(2, 3U);
}

void FrameBufferAdapterTester::testRejectsInvalidPackedFrame() {
    this->fillPixels(0);
    this->packFrame(5U, WIDTH, HEIGHT);

    // Truncated header
    this->sendPacked(HEADER_SIZE - 1);
    // Payload one byte short and one byte long
    this->sendPacked(HEADER_SIZE + PIXELS - 1);
    this->sendPacked(HEADER_SIZE + PIXELS + 1);
    // Width beyond the downsampled frame
    this->m_packed[5] = 0xFFU;
    this->sendPacked(HEADER_SIZE + PIXELS);
    // Zero height
    this->m_packed[5] = static_cast<U8>(WIDTH >> 8);
    this->m_packed[7] = 0;
    this->m_packed[8] = 0;
    this->sendPacked(HEADER_SIZE);

    ASSERT_from_frameOut_SIZE(0);
    ASSERT_EVENTS_FrameRejected_SIZE(5);
    ASSERT_EVENTS_FrameRejected(0, FrameBufferStatus::SHORT_BUFFER, 0, 0, HEADER_SIZE - 1);
    ASSERT_EVENTS_FrameRejected(1, FrameBufferStatus::SIZE_MISMATCH, WIDTH, HEIGHT, HEADER_SIZE + PIXELS - 1);
    ASSERT_EVENTS_FrameRejected(2, FrameBufferStatus::SIZE_MISMATCH, WIDTH, HEIGHT, HEADER_SIZE + PIXELS + 1);
    ASSERT_EVENTS_FrameRejected(3, FrameBufferStatus::BAD_GEOMETRY, static_cast<U16>(0xFF00U | (WIDTH & 0xFFU)), HEIGHT,
                                HEADER_SIZE + PIXELS);
    ASSERT_EVENTS_FrameRejected(4, FrameBufferStatus::BAD_GEOMETRY, WIDTH, 0, HEADER_SIZE);
    ASSERT_TLM_FramesRejected(4, 5U);
    ASSERT_TLM_FramesUnpacked_SIZE(0);
}

void FrameBufferAdapterTester::testRejectsInvalidPackedPalette() {
    this->packPalette(makePalette(3U));
    this->sendPacked(PALETTE_SIZE - 1);
    this->sendPacked(PALETTE_SIZE + 1);

    ASSERT_from_paletteOut_SIZE(0);
    ASSERT_EVENTS_PaletteRejected_SIZE(2);
    ASSERT_EVENTS_PaletteRejected(0, FrameBufferStatus::SIZE_MISMATCH, PALETTE_SIZE - 1);
    ASSERT_EVENTS_PaletteRejected(1, FrameBufferStatus::SIZE_MISMATCH, PALETTE_SIZE + 1);
    ASSERT_TLM_PalettesRejected(1, 2U);
}

// ----------------------------------------------------------------------
// Echo relay
// ----------------------------------------------------------------------

void FrameBufferAdapterTester::testEchoRelaysAndReturns() {
    Fw::Buffer buffer(this->m_packed, PALETTE_SIZE);
    buffer.setContext(0x1234U);
    this->invoke_to_echoIn(0, buffer);
    ASSERT_from_echoOut_SIZE(1);
    ASSERT_from_echoOut(0, buffer);
    ASSERT_from_echoReturn_SIZE(0);

    Fw::Buffer returned = this->fromPortHistory_echoOut->at(0).fwBuffer;
    this->invoke_to_echoOutReturn(0, returned);
    ASSERT_from_echoOut_SIZE(1);
    ASSERT_from_echoReturn_SIZE(1);
    ASSERT_from_echoReturn(0, buffer);
    ASSERT_EVENTS_SIZE(0);
    ASSERT_TLM_SIZE(0);
}

void FrameBufferAdapterTester::testEchoReturnsWhenUnconnected() {
    FrameBufferAdapter unconnected("UnconnectedEcho");
    unconnected.init(TEST_INSTANCE_ID);
    unconnected.set_echoReturn_OutputPort(0, this->get_from_echoReturn(0));

    Fw::Buffer buffer(this->m_packed, PALETTE_SIZE);
    unconnected.get_echoIn_InputPort(0)->invoke(buffer);
    ASSERT_from_echoOut_SIZE(0);
    ASSERT_from_echoReturn_SIZE(1);
    ASSERT_from_echoReturn(0, buffer);
}

}  // namespace Components
