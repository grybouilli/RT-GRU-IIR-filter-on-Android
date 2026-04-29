#pragma once

#include <GRUInferenceMethods/GeneralInferenceParams.hpp>
#include <GRUInferenceMethods/IEParams.hpp>
#include <GRUInferenceMethods/Ort/GRUOrtInference.hpp>
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
        switch (m_chosen_method) {
            case SupportedInferenceEngines::Ort:
                m_ort_method =
                    std::make_unique<GRUOrtInference<IIRGRU>>(gru,
                                                              gparams,
                                                              ieparams);
                break;

            default:
                break;
        }
    }
    bool run(const float* audio_in, float* audio_out) {
        switch (m_chosen_method) {
            case SupportedInferenceEngines::Ort:
                return m_ort_method->run(audio_in, audio_out);
                break;

            default:
                break;
        }
    }

   private:
    SupportedInferenceEngines                m_chosen_method;
    std::unique_ptr<GRUOrtInference<IIRGRU>> m_ort_method;
};