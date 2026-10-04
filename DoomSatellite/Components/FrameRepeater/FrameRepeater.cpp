// ======================================================================
// \title  FrameRepeater.cpp
// \brief  cpp file for FrameRepeater component implementation class
// ======================================================================

#include "DoomSatellite/Components/FrameRepeater/FrameRepeater.hpp"

namespace Components {

FrameRepeater::FrameRepeater(const char* const compName)
    : FrameRepeaterComponentBase(compName), m_framesRepeated(0), m_palettesRepeated(0) {}

FrameRepeater::~FrameRepeater() {}

void FrameRepeater::frameIn_handler(FwIndexType portNum, U32 frameNumber, U16 width, U16 height, Fw::Buffer& pixels) {
    for (FwIndexType i = 0; i < NUM_FRAMEOUT_OUTPUT_PORTS; i++) {
        if (this->isConnected_frameOut_OutputPort(i)) {
            // Each consumer gets its own view of the caller's storage so none can redirect the others
            Fw::Buffer view(pixels);
            this->frameOut_out(i, frameNumber, width, height, view);
        }
    }
    this->m_framesRepeated++;
    this->tlmWrite_FramesRepeated(this->m_framesRepeated);
}

void FrameRepeater::paletteIn_handler(FwIndexType portNum, const Doom::Palette& palette) {
    for (FwIndexType i = 0; i < NUM_PALETTEOUT_OUTPUT_PORTS; i++) {
        if (this->isConnected_paletteOut_OutputPort(i)) {
            this->paletteOut_out(i, palette);
        }
    }
    this->m_palettesRepeated++;
    this->tlmWrite_PalettesRepeated(this->m_palettesRepeated);
}

}  // namespace Components
