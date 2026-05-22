#pragma once

#include <oboe/Oboe.h>

#include <AudioParams.hpp>
#include <ModelBinding.hpp>
#include <ModelInferenceMethods/GeneralInferenceParams.hpp>
#include <ModelInferenceMethods/IEParams.hpp>
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
//   through the GRU filter via ModelBinding (zero hot-path allocations), and
//   writes filtered samples back to Oboe.
// ---------------------------------------------------------------------------
class Player : public oboe::AudioStreamDataCallback {
   public:
    bool debug;

   public:
    Player(int32_t       sample_rate,
           int32_t       channels,
           audio_buffer& buffer,
           const bool    dbg = false) :
        m_sample_rate{sample_rate},
        m_channels{channels},
        m_buffer{buffer},
        debug{dbg} {}

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream* /*stream*/,
                                          void*   audio_data,
                                          int32_t num_frames) override {
        using namespace std::chrono;
        audio_sample_t* out         = static_cast<audio_sample_t*>(audio_data);
        const int32_t   num_samples = num_frames * m_channels;

        // Read raw input from the shared ring-buffer
        m_buffer.read(out, num_samples);

        if (debug) {
            m_recorded_signal.insert(m_recorded_signal.end(),
                                     out,
                                     out + num_samples);
        }

        return oboe::DataCallbackResult::Continue;
    }

    void dump_debug_output(const std::string& filename) {
        npy::npy_data<audio_sample_t> d;
        d.data  = m_recorded_signal;
        d.shape = {m_recorded_signal.size()};
        npy::write_npy(filename, d);
    }

   private:
    int32_t       m_sample_rate;
    int32_t       m_channels;
    audio_buffer& m_buffer;

    std::vector<audio_sample_t> m_recorded_signal;
};