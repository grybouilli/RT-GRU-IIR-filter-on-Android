#pragma once

#include <GRUInferenceMethods/Anira/AniraGRUInference.hpp>
#include <GRUInferenceMethods/GeneralInferenceParams.hpp>
#include <GRUInferenceMethods/IEParams.hpp>
#include <GRUInferenceMethods/Ort/OrtGRUInference.hpp>
#include <IIRGRUInfo.hpp>
#include <memory>

template <IsIIRGRUInfo IIRGRU>
class GRUBinding {
   public:
    template <typename IEParams>
    GRUBinding(const IIRGRU&                gru,
               const GeneralInferenceParams gparams,
               const IEParams&              ieparams) :
        m_chosen_method{gparams.chosen_engine} {
        if constexpr (std::is_same_v<IEParams, OrtParams>) {
            m_inference_method =
                std::make_unique<OrtGRUInference<IIRGRU>>(gru,
                                                          gparams,
                                                          ieparams);
        } else if constexpr (std::is_same_v<IEParams, AniraParams>) {
            m_inference_method =
                std::make_unique<AniraGRUInference<IIRGRU>>(gru,
                                                            gparams,
                                                            ieparams);
        }
    }
    bool run(float* audio_data, const size_t num_samples) {
        return m_inference_method->run(audio_data, num_samples);
    }

   private:
    SupportedInferenceEngines                       m_chosen_method;
    std::unique_ptr<GRUInferenceMethodBase<IIRGRU>> m_inference_method;
};