#pragma once

#include <oboe/Oboe.h>

#include <AudioParams.hpp>
#include <GRUBinding.hpp>
#include <GRUInferenceMethods/GeneralInferenceParams.hpp>
#include <GRUInferenceMethods/IEParams.hpp>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <npy.hpp>
#include <vector>
// ---------------------------------------------------------------------------
// Player
//   Oboe output callback.  Reads raw audio from SharedAudioBuffer, runs it
//   through the GRU filter via GRUBinding (zero hot-path allocations), and
//   writes filtered samples back to Oboe.
// ---------------------------------------------------------------------------
template <IsIIRGRUInfo IIRGRU>
class Player : public oboe::AudioStreamDataCallback {
   public:
    bool debug;
    bool profiling;

   public:
    template <typename IEParams>
    Player(const IIRGRU&                gru,
           const GeneralInferenceParams gparams,
           const IEParams&              ieparams,
           int32_t                      sample_rate,
           int32_t                      channels,
           audio_buffer&                buffer,
           const bool                   dbg   = false,
           const bool                   prflg = false) :
        m_sample_rate{sample_rate},
        m_channels{channels},
        m_buffer{buffer},
        m_gru_binding{gru, gparams, ieparams},
        m_expected_frames{static_cast<int32_t>(gru.buffer_size())},
        debug{dbg},
        profiling{prflg} {}

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream* /*stream*/,
                                          void*   audio_data,
                                          int32_t num_frames) override {
        using namespace std::chrono;
        audio_sample_t* out         = static_cast<audio_sample_t*>(audio_data);
        const int32_t   num_samples = num_frames * m_channels;

        // Read raw input from the shared ring-buffer
        m_buffer.read(out, num_samples);

        // Guard: GRU binding is sized for a fixed buffer_size; skip inference
        // if Oboe gives us an unexpected frame count to avoid UB.
        if (num_frames != m_expected_frames) {
            fprintf(stderr,
                    "Player: unexpected frame count %d (expected %d), passing "
                    "through\n",
                    num_frames,
                    m_expected_frames);
            return oboe::DataCallbackResult::Continue;
        }

        bool                                                ret;
        decltype(std::chrono::high_resolution_clock::now()) start, end;
        if (profiling) {
            start = std::chrono::high_resolution_clock::now();
        }

        // Run inference - writes filtered samples back into `out` in-place
        ret = m_gru_binding.run(out, num_samples);

        if (profiling) {
            end = std::chrono::high_resolution_clock::now();
            m_recorded_performances.push_back(
                duration_cast<milliseconds>(end - start).count());
        }
        if (ret && debug) {
            // Record output for offline dump
            m_recorded_signal.insert(m_recorded_signal.end(),
                                     out,
                                     out + num_samples);
        }

        return oboe::DataCallbackResult::Continue;
    }

    void dump_debug(const std::string& filename) {
        npy::npy_data<audio_sample_t> d;
        d.data  = m_recorded_signal;
        d.shape = {m_recorded_signal.size()};
        npy::write_npy(filename, d);
    }

    void dump_profiling(const std::string& filename) {
        npy::npy_data<double> d;
        d.data  = m_recorded_performances;
        d.shape = {m_recorded_performances.size()};
        npy::write_npy(filename, d);
    }

   private:
    int32_t       m_sample_rate;
    int32_t       m_channels;
    audio_buffer& m_buffer;

    GRUBinding<IIRGRU> m_gru_binding;

    int32_t m_expected_frames;

    std::vector<audio_sample_t> m_recorded_signal;

    std::vector<double> m_recorded_performances;
};