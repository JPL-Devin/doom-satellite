// ======================================================================
// \title  FrameBufferAdapter.cpp
// \brief  cpp file for FrameBufferAdapter component implementation class
// ======================================================================

#include "DoomSatellite/Components/FrameBufferAdapter/FrameBufferAdapter.hpp"

#include <cstring>

namespace Components {

FrameBufferAdapter::FrameBufferAdapter(const char* const compName)
    : FrameBufferAdapterComponentBase(compName),
      m_frameStorage{},
      m_paletteStorage{},
      m_frameLent(false),
      m_paletteLent(false),
      m_packedRejected(0),
      m_framesPacked(0),
      m_framesUnpacked(0),
      m_framesRejected(0),
      m_palettesPacked(0),
      m_palettesUnpacked(0),
      m_palettesRejected(0) {}

FrameBufferAdapter::~FrameBufferAdapter() {}

FrameBufferStatus FrameBufferAdapter::checkGeometry(U16 width, U16 height) {
    if ((width == 0) || (height == 0) || (width > Doom::DOWNSAMPLED_WIDTH) || (height > Doom::DOWNSAMPLED_HEIGHT)) {
        return FrameBufferStatus::BAD_GEOMETRY;
    }
    return FrameBufferStatus::OK;
}

void FrameBufferAdapter::rejectPacked(FrameBufferStatus reason, FwSizeType size) {
    this->m_packedRejected++;
    this->log_WARNING_LO_PackedRejected(reason, size);
    this->tlmWrite_PackedRejected(this->m_packedRejected);
}

void FrameBufferAdapter::rejectFrame(FrameBufferStatus reason, U16 width, U16 height, FwSizeType size) {
    this->m_framesRejected++;
    this->log_WARNING_LO_FrameRejected(reason, width, height, size);
    this->tlmWrite_FramesRejected(this->m_framesRejected);
}

void FrameBufferAdapter::rejectPalette(FrameBufferStatus reason, FwSizeType size) {
    this->m_palettesRejected++;
    this->log_WARNING_LO_PaletteRejected(reason, size);
    this->tlmWrite_PalettesRejected(this->m_palettesRejected);
}

// ----------------------------------------------------------------------
// Pack
// ----------------------------------------------------------------------

FrameBufferStatus FrameBufferAdapter::checkFrameIn(U16 width, U16 height, const Fw::Buffer& pixels) const {
    if (checkGeometry(width, height) != FrameBufferStatus::OK) {
        return FrameBufferStatus::BAD_GEOMETRY;
    }
    if (pixels.getData() == nullptr) {
        return FrameBufferStatus::NULL_BUFFER;
    }
    if (pixels.getSize() < static_cast<FwSizeType>(width) * static_cast<FwSizeType>(height)) {
        return FrameBufferStatus::SHORT_BUFFER;
    }
    if (this->m_frameLent) {
        return FrameBufferStatus::BUFFER_IN_USE;
    }
    return FrameBufferStatus::OK;
}

void FrameBufferAdapter::frameIn_handler(FwIndexType portNum,
                                         U32 frameNumber,
                                         U16 width,
                                         U16 height,
                                         Fw::Buffer& pixels) {
    if (!this->isConnected_packedOut_OutputPort(0)) {
        return;
    }
    const FrameBufferStatus check = this->checkFrameIn(width, height, pixels);
    if (check != FrameBufferStatus::OK) {
        this->rejectFrame(check, width, height, pixels.getSize());
        return;
    }
    this->log_WARNING_LO_FrameRejected_ThrottleClear();

    const FwSizeType frameBytes = static_cast<FwSizeType>(width) * static_cast<FwSizeType>(height);
    FW_ASSERT(FRAME_HEADER_SIZE + frameBytes <= sizeof(this->m_frameStorage), static_cast<FwAssertArgType>(frameBytes));
    Fw::ExternalSerializeBuffer header(this->m_frameStorage, FRAME_HEADER_SIZE);
    Fw::SerializeStatus status = header.serializeFrom(FrameBufferKind(FrameBufferKind::FRAME));
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
    status = header.serializeFrom(frameNumber);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
    status = header.serializeFrom(width);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
    status = header.serializeFrom(height);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
    (void)::memcpy(&this->m_frameStorage[FRAME_HEADER_SIZE], pixels.getData(), static_cast<size_t>(frameBytes));

    Fw::Buffer packed(this->m_frameStorage, FRAME_HEADER_SIZE + frameBytes);
    this->m_frameLent = true;
    this->packedOut_out(0, packed);
    this->m_framesPacked++;
    this->tlmWrite_FramesPacked(this->m_framesPacked);
}

void FrameBufferAdapter::paletteIn_handler(FwIndexType portNum, const Doom::Palette& palette) {
    if (!this->isConnected_packedOut_OutputPort(0)) {
        return;
    }
    if (this->m_paletteLent) {
        this->rejectPalette(FrameBufferStatus::BUFFER_IN_USE, PACKED_PALETTE_SIZE);
        return;
    }
    this->log_WARNING_LO_PaletteRejected_ThrottleClear();

    Fw::ExternalSerializeBuffer serializer(this->m_paletteStorage, sizeof(this->m_paletteStorage));
    Fw::SerializeStatus status = serializer.serializeFrom(FrameBufferKind(FrameBufferKind::PALETTE));
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
    status = serializer.serializeFrom(palette);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));

    Fw::Buffer packed(this->m_paletteStorage, PACKED_PALETTE_SIZE);
    this->m_paletteLent = true;
    this->packedOut_out(0, packed);
    this->m_palettesPacked++;
    this->tlmWrite_PalettesPacked(this->m_palettesPacked);
}

void FrameBufferAdapter::packedOutReturn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    if (fwBuffer.getData() == this->m_frameStorage) {
        FW_ASSERT(this->m_frameLent);
        this->m_frameLent = false;
    } else {
        FW_ASSERT(fwBuffer.getData() == this->m_paletteStorage);
        FW_ASSERT(this->m_paletteLent);
        this->m_paletteLent = false;
    }
}

// ----------------------------------------------------------------------
// Unpack
// ----------------------------------------------------------------------

FrameBufferStatus FrameBufferAdapter::unpackFrame(const Fw::Buffer& fwBuffer, U16& width, U16& height) {
    if (fwBuffer.getSize() < FRAME_HEADER_SIZE) {
        return FrameBufferStatus::SHORT_BUFFER;
    }
    Fw::ExternalSerializeBuffer header(fwBuffer.getData(), FRAME_HEADER_SIZE);
    Fw::SerializeStatus status = header.setBuffLen(FRAME_HEADER_SIZE);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
    status = header.moveDeserToOffset(KIND_SIZE);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
    U32 frameNumber = 0;
    status = header.deserializeTo(frameNumber);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
    status = header.deserializeTo(width);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
    status = header.deserializeTo(height);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));

    if (checkGeometry(width, height) != FrameBufferStatus::OK) {
        return FrameBufferStatus::BAD_GEOMETRY;
    }
    const FwSizeType frameBytes = static_cast<FwSizeType>(width) * static_cast<FwSizeType>(height);
    if (fwBuffer.getSize() != FRAME_HEADER_SIZE + frameBytes) {
        return FrameBufferStatus::SIZE_MISMATCH;
    }
    if (this->isConnected_frameOut_OutputPort(0)) {
        Fw::Buffer pixels(&fwBuffer.getData()[FRAME_HEADER_SIZE], frameBytes);
        this->frameOut_out(0, frameNumber, width, height, pixels);
    }
    return FrameBufferStatus::OK;
}

FrameBufferStatus FrameBufferAdapter::unpackPalette(const Fw::Buffer& fwBuffer) {
    if (fwBuffer.getSize() != PACKED_PALETTE_SIZE) {
        return FrameBufferStatus::SIZE_MISMATCH;
    }
    Fw::ExternalSerializeBuffer deserializer(fwBuffer.getData(), PACKED_PALETTE_SIZE);
    Fw::SerializeStatus status = deserializer.setBuffLen(PACKED_PALETTE_SIZE);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
    status = deserializer.moveDeserToOffset(KIND_SIZE);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
    Doom::Palette palette;
    status = deserializer.deserializeTo(palette);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
    if (this->isConnected_paletteOut_OutputPort(0)) {
        this->paletteOut_out(0, palette);
    }
    return FrameBufferStatus::OK;
}

void FrameBufferAdapter::handlePackedFrame(const Fw::Buffer& fwBuffer) {
    U16 width = 0;
    U16 height = 0;
    const FrameBufferStatus status = this->unpackFrame(fwBuffer, width, height);
    if (status == FrameBufferStatus::OK) {
        this->log_WARNING_LO_FrameRejected_ThrottleClear();
        this->m_framesUnpacked++;
        this->tlmWrite_FramesUnpacked(this->m_framesUnpacked);
    } else {
        this->rejectFrame(status, width, height, fwBuffer.getSize());
    }
}

void FrameBufferAdapter::handlePackedPalette(const Fw::Buffer& fwBuffer) {
    const FrameBufferStatus status = this->unpackPalette(fwBuffer);
    if (status == FrameBufferStatus::OK) {
        this->log_WARNING_LO_PaletteRejected_ThrottleClear();
        this->m_palettesUnpacked++;
        this->tlmWrite_PalettesUnpacked(this->m_palettesUnpacked);
    } else {
        this->rejectPalette(status, fwBuffer.getSize());
    }
}

void FrameBufferAdapter::packedIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    if (fwBuffer.getData() == nullptr) {
        this->rejectPacked(FrameBufferStatus::NULL_BUFFER, fwBuffer.getSize());
    } else if (fwBuffer.getSize() < KIND_SIZE) {
        this->rejectPacked(FrameBufferStatus::SHORT_BUFFER, fwBuffer.getSize());
    } else {
        Fw::ExternalSerializeBuffer kindReader(fwBuffer.getData(), KIND_SIZE);
        Fw::SerializeStatus status = kindReader.setBuffLen(KIND_SIZE);
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
        FrameBufferKind kind;
        status = kindReader.deserializeTo(kind);
        if ((status == Fw::FW_SERIALIZE_OK) && (kind == FrameBufferKind::FRAME)) {
            this->handlePackedFrame(fwBuffer);
        } else if ((status == Fw::FW_SERIALIZE_OK) && (kind == FrameBufferKind::PALETTE)) {
            this->handlePackedPalette(fwBuffer);
        } else {
            this->rejectPacked(FrameBufferStatus::BAD_KIND, fwBuffer.getSize());
        }
    }
    this->packedInReturn_out(0, fwBuffer);
}

// ----------------------------------------------------------------------
// Echo relay
// ----------------------------------------------------------------------

void FrameBufferAdapter::echoIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    if (this->isConnected_echoOut_OutputPort(0)) {
        this->echoOut_out(0, fwBuffer);
    } else {
        this->echoReturn_out(0, fwBuffer);
    }
}

void FrameBufferAdapter::echoOutReturn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    this->echoReturn_out(0, fwBuffer);
}

}  // namespace Components
