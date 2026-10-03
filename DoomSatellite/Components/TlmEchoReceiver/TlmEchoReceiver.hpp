// ======================================================================
// \title  TlmEchoReceiver.hpp
// \brief  hpp file for TlmEchoReceiver component implementation class
// ======================================================================

#ifndef Components_TlmEchoReceiver_HPP
#define Components_TlmEchoReceiver_HPP

#include "DoomSatellite/Components/TlmEchoReceiver/TlmEchoReceiverComponentAc.hpp"

#include <atomic>

namespace Components {

class TlmEchoReceiver final : public TlmEchoReceiverComponentBase {
  public:
    //! Construct TlmEchoReceiver object
    explicit TlmEchoReceiver(const char* const compName  //!< The component name
    );

    //! Destroy TlmEchoReceiver object
    ~TlmEchoReceiver();

  private:
    //! Handler implementation for serialIn
    void serialIn_handler(FwIndexType portNum,          //!< The port number
                          Fw::LinearBufferBase& buffer  //!< The serialization buffer
                          ) override;

    //! Handler implementation for schedIn
    void schedIn_handler(FwIndexType portNum,  //!< The port number
                         U32 context           //!< The call order
                         ) override;

    std::atomic<U32> m_received;  //!< Samples received since the last report
    std::atomic<U32> m_errors;    //!< Deserialization failures since the last report
    U32 m_total;                  //!< Samples received since startup
};

}  // namespace Components

#endif
