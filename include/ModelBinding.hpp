#pragma once

#include <IIRGRUInfo.hpp>
#include <ModelInferenceMethods/GRUInferenceMethods/Anira/AniraGRUInference.hpp>
#include <ModelInferenceMethods/GRUInferenceMethods/Ort/OrtGRUInference.hpp>
#include <ModelInferenceMethods/GeneralInferenceParams.hpp>
#include <ModelInferenceMethods/IEParams.hpp>
#include <memory>

template <typename ModelInfo>
class ModelBinding {
   public:
    template <typename IEParams>
    ModelBinding(const ModelInfo&             gru,
                 const GeneralInferenceParams gparams,
                 const IEParams&              ieparams) :
        m_chosen_method{gparams.chosen_engine} {
        if constexpr (IsIIRGRUInfo<ModelInfo>) {
            if constexpr (std::is_same_v<IEParams, OrtParams>) {
                m_inference_method =
                    std::make_unique<OrtGRUInference<ModelInfo>>(gru,
                                                                 gparams,
                                                                 ieparams);
            } else if constexpr (std::is_same_v<IEParams, AniraParams>) {
                m_inference_method =
                    std::make_unique<AniraGRUInference<ModelInfo>>(gru,
                                                                   gparams,
                                                                   ieparams);
            }
        }
    }
    bool run(float* audio_data, const size_t num_samples) {
        return m_inference_method->run(audio_data, num_samples);
    }

   private:
    SupportedInferenceEngines                            m_chosen_method;
    std::unique_ptr<ModelInferenceMethodBase<ModelInfo>> m_inference_method;
};