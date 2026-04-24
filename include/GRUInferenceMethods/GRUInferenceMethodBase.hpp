#pragma once

#include <GRUInferenceMethods/GeneralInferenceParams.hpp>
#include <GRUInferenceMethods/IEParams.hpp>
#include <IIRGRUInfo.hpp>

template <IsIIRGRUInfo IIRGRU>
class GRUInferenceMethodBase {
   public:
    template <typename IEParams>
    explicit GRUInferenceMethodBase(const IIRGRU&                gru,
                                    const GeneralInferenceParams gparams,
                                    const IEParams&              ieparams) :
        m_gru{gru} {}

    virtual bool run(const float* audio_in, float* audio_out) = 0;

   protected:
    IIRGRU m_gru;
};