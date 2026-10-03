// ======================================================================
// \title  TlmSplitter.cpp
// \brief  cpp file for TlmSplitter component implementation class
// ======================================================================

#include "DoomSatellite/Components/TlmSplitter/TlmSplitter.hpp"

#include <Fw/Types/Assert.hpp>
#include <Fw/Types/Serializable.hpp>

namespace Components {

TlmSplitter::TlmSplitter(const char* const compName) : TlmSplitterComponentBase(compName) {}

TlmSplitter::~TlmSplitter() {}

void TlmSplitter::tlmIn_handler(FwIndexType portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val) {
    if (this->isConnected_tlmOut_OutputPort(0)) {
        this->tlmOut_out(0, id, timeTag, val);
    }
    if (this->isConnected_echoOut_OutputPort(0)) {
        U8 storage[sizeof(FwChanIdType) + Fw::Time::SERIALIZED_SIZE + sizeof(FwSizeStoreType) +
                   FW_TLM_BUFFER_MAX_SIZE]{};
        Fw::ExternalSerializeBuffer serializer(storage, sizeof(storage));
        Fw::SerializeStatus status = serializer.serializeFrom(id);
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
        status = serializer.serializeFrom(timeTag);
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
        status = serializer.serializeFrom(val);
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, static_cast<FwAssertArgType>(status));
        (void)this->echoOut_out(0, serializer);
    }
}

}  // namespace Components
