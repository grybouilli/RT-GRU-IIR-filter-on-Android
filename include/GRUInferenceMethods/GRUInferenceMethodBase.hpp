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
                                    const IEParams&              ieparams) {}

    virtual ~GRUInferenceMethodBase() {}
    virtual bool run(float* audio, const size_t samples) = 0;
};