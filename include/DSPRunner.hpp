#pragma once

#include <AudioParams.hpp>
#include <ModelBinding.hpp>
#include <ModelInferenceMethods/GeneralInferenceParams.hpp>
#include <SharedAudioBuffer.hpp>
#include <array>
#include <npy.hpp>
#include <ratio>
#include <thread>
#include <vector>

template <typename ModelInfo>
class DSPRunner {
    using Clock = std::chrono::high_resolution_clock;
    template <class Duration>
    using TimePoint = std::chrono::time_point<Clock, Duration>;

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
        m_profiling{gparams.profiling},
        m_profiling_data_file{gparams.profiling_file},
        m_thread{[this, gparams] {
            while (m_alive) {
                while (m_samples_to_process.size() >=
                       ModelInfo::buffer_size()) {
                    m_inference_running = true;

                    std::array<audio_sample_t, ModelInfo::buffer_size()> buffer;
                    m_samples_to_process.read(buffer.data(), buffer.size());

                    TimePoint<std::chrono::nanoseconds> start;
                    if (gparams.profiling)
                        start = std::chrono::high_resolution_clock::now();
                    m_dsp.run(buffer.data(), ModelInfo::buffer_size());
                    if (gparams.profiling) {
                        const auto end =
                            std::chrono::high_resolution_clock::now();
                        m_latencies.push_back(
                            std::chrono::duration<double, std::nano>(end -
                                                                     start)
                                .count());
                    }
                    m_processed_samples.write(buffer.data(), buffer.size());

                    m_inference_running = false;
                    m_inference_running.notify_all();
                }
                std::this_thread::yield();
            }
        }} {}

    void dump(const std::string filename) {
        npy::npy_data<double> d;
        d.data  = m_latencies;
        d.shape = {m_latencies.size()};

        npy::write_npy(filename, d);
    }

    ~DSPRunner() {
        m_alive = false;
        m_inference_running.wait(true);
        if (m_profiling) dump(m_profiling_data_file);
    }

   private:
    SharedAudioBuffer<audio_sample_t>& m_samples_to_process;
    SharedAudioBuffer<audio_sample_t>& m_processed_samples;
    ModelBinding<ModelInfo>            m_dsp;
    std::atomic_bool                   m_alive;
    std::atomic_bool                   m_inference_running;
    bool                               m_profiling;
    std::string                        m_profiling_data_file;
    std::vector<double>                m_latencies;
    std::jthread                       m_thread;
};