// ======================================================================
// \title  TlmEchoReceiver.cpp
// \brief  cpp file for TlmEchoReceiver component implementation class
// ======================================================================

#include "DoomSatellite/Components/TlmEchoReceiver/TlmEchoReceiver.hpp"

namespace Components {

TlmEchoReceiver::TlmEchoReceiver(const char* const compName)
    : TlmEchoReceiverComponentBase(compName), m_received(0), m_errors(0), m_total(0) {}

TlmEchoReceiver::~TlmEchoReceiver() {}

void TlmEchoReceiver::serialIn_handler(FwIndexType portNum, Fw::LinearBufferBase& buffer) {
    FwChanIdType id = 0;
    Fw::Time timeTag;
    Fw::TlmBuffer val;
    Fw::SerializeStatus status = buffer.deserializeTo(id);
    if (status == Fw::FW_SERIALIZE_OK) {
        status = buffer.deserializeTo(timeTag);
    }
    if (status == Fw::FW_SERIALIZE_OK) {
        status = buffer.deserializeTo(val);
    }
    if (status != Fw::FW_SERIALIZE_OK) {
        this->m_errors++;
        return;
    }
    this->m_received++;
    if (this->isConnected_tlmOut_OutputPort(0)) {
        this->tlmOut_out(0, id, timeTag, val);
    }
}

void TlmEchoReceiver::schedIn_handler(FwIndexType portNum, U32 context) {
    const U32 received = this->m_received.exchange(0);
    const U32 errors = this->m_errors.exchange(0);
    if ((received != 0) || (errors != 0)) {
        this->m_total += received;
        this->log_ACTIVITY_LO_EchoStatus(received, errors, this->m_total);
    }
}

}  // namespace Components
