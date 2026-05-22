#pragma once

#include <AudioParams.hpp>
#include <ModelBinding.hpp>
#include <ModelInferenceMethods/GeneralInferenceParams.hpp>
#include <SharedAudioBuffer.hpp>
#include <thread>

template <typename ModelInfo>
class DSPRunner {
   public:
    template <typename IEParams>
    DSPRunner(SharedAudioBuffer<audio_sample_t>& input_samples,
              SharedAudioBuffer<audio_sample_t>& output_samples,
              const ModelInfo&                   model,
              const GeneralInferenceParams       gparams,
              const IEParams&                    ieparams) :
        m_samples_to_process{input_samples},
        m_processed_samples{output_samples},
        m_dsp{model, gparams, ieparams},
        m_alive{true},
        m_thread{[this] {
            while (m_alive) {
                if (m_samples_to_process.size() > ModelInfo::buffer_size()) {
                    m_inference_running = true;

                    std::array<audio_sample_t, ModelInfo::buffer_size()> buffer;
                    m_samples_to_process.read(buffer.data(), buffer.size());

                    m_dsp.run(buffer.data(), ModelInfo::buffer_size());

                    m_processed_samples.write(buffer.data(), buffer.size());

                    m_inference_running = false;
                    m_inference_running.notify_all();
                }
                std::this_thread::sleep_for(std::chrono::microseconds(500));
            }
        }} {}

    ~DSPRunner() { m_alive = false; }

   private:
    SharedAudioBuffer<audio_sample_t>& m_samples_to_process;
    SharedAudioBuffer<audio_sample_t>& m_processed_samples;
    ModelBinding<ModelInfo>            m_dsp;
    std::atomic_bool                   m_alive;
    std::atomic_bool                   m_inference_running;
    std::jthread                       m_thread;
};