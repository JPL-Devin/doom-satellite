// ======================================================================
// \title  TlmSplitter.hpp
// \brief  hpp file for TlmSplitter component implementation class
// ======================================================================

#ifndef Components_TlmSplitter_HPP
#define Components_TlmSplitter_HPP

#include "DoomSatellite/Components/TlmSplitter/TlmSplitterComponentAc.hpp"

namespace Components {

class TlmSplitter final : public TlmSplitterComponentBase {
  public:
    //! Construct TlmSplitter object
    explicit TlmSplitter(const char* const compName  //!< The component name
    );

    //! Destroy TlmSplitter object
    ~TlmSplitter();

  private:
    //! Handler implementation for tlmIn
    void tlmIn_handler(FwIndexType portNum,  //!< The port number
                       FwChanIdType id,      //!< Telemetry Channel ID
                       Fw::Time& timeTag,    //!< Time Tag
                       Fw::TlmBuffer& val    //!< Buffer containing serialized telemetry value
                       ) override;
};

}  // namespace Components

#endif
