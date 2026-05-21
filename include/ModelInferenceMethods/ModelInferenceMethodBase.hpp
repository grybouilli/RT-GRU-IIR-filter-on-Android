#pragma once

#include <IIRGRUInfo.hpp>
#include <ModelInferenceMethods/GeneralInferenceParams.hpp>
#include <ModelInferenceMethods/IEParams.hpp>

template <typename ModelInfo>
class ModelInferenceMethodBase {
   public:
    template <typename IEParams>
    explicit ModelInferenceMethodBase(const ModelInfo&             gru,
                                      const GeneralInferenceParams gparams,
                                      const IEParams&              ieparams) {}

    virtual ~ModelInferenceMethodBase() {}
    virtual bool run(float* audio, const size_t samples) = 0;
};