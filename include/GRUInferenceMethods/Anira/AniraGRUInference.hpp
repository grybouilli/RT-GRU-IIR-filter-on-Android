#pragma once

#include <anira/anira.h>

#include <GRUInferenceMethods/Anira/PrePostGRUProcessor.hpp>
#include <GRUInferenceMethods/GRUInferenceMethodBase.hpp>
#include <GRUInferenceMethods/GeneralInferenceParams.hpp>
#include <GRUInferenceMethods/IEParams.hpp>
#include <magic_enum/magic_enum.hpp>

template <IsIIRGRUInfo IIRGRU>
class AniraGRUInference final : public GRUInferenceMethodBase<IIRGRU> {
   public:
    AniraGRUInference(const IIRGRU&                gru,
                      const GeneralInferenceParams gparams,
                      const AniraParams&           ieparams) :
        GRUInferenceMethodBase<IIRGRU>(gru, gparams, ieparams),
        m_chosen_backend{
            magic_enum::enum_cast<anira::InferenceBackend>(ieparams.backend)
                .value_or(anira::InferenceBackend::ONNX)},
        m_inference_config(
            {{gparams.model_filename, m_chosen_backend}},
            {anira::TensorShape(
                {{gru.batch_size(), gru.buffer_size(), gru.input_size()},
                 {gru.num_layers(), gru.batch_size(), gru.hidden_size()}},
                {{gru.batch_size(), gru.buffer_size(), 1},
                 {gru.num_layers(), gru.batch_size(), gru.hidden_size()}})},

            anira::ProcessingSpec(
                // preprocess_input_channels: tensor0 has 1 audio ch, tensor1
                // (hidden) has 1 ch
                {1, 1},
                // postprocess_output_channels: tensor0 (audio out) has 1 ch,
                // tensor1 (hidden out) has 1 ch
                {1, 1},
                // preprocess_input_size: tensor0 streams buffer_size samples,
                // tensor1 is NON-STREAMABLE
                {static_cast<size_t>(gru.buffer_size()), 0},
                // postprocess_output_size: tensor0 produces buffer_size
                // samples, tensor1 is NON-STREAMABLE
                {static_cast<size_t>(gru.buffer_size()), 0}),
            ieparams.model_latency),
        m_pp_processor{m_inference_config,
                       static_cast<size_t>(gru.num_layers()),
                       static_cast<size_t>(gru.hidden_size()),
                       gparams.Fc_normed},
        m_inference_handler{m_pp_processor, m_inference_config} {
        m_inference_handler.prepare(
            {static_cast<float>(gru.buffer_size()), 48000.f});
        m_inference_handler.set_inference_backend(
            anira::InferenceBackend::ONNX);
        printf("AniraGRUInference class initialized with latency set to %f\n",
               ieparams.model_latency);
    }

    bool run(float* audio_data, const size_t num_samples) override {
        m_inference_handler.process(&audio_data, num_samples);

        return true;
    }

   private:
    anira::InferenceBackend m_chosen_backend;
    anira::InferenceConfig  m_inference_config;
    PrePostGRUProcessor     m_pp_processor;
    anira::InferenceHandler m_inference_handler;
};