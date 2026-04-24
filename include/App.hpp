#pragma once

#include <onnxruntime_cxx_api.h>
#include <signal.h>

#include <AudioParams.hpp>
#include <GRUInferenceMethods/GeneralInferenceParams.hpp>
#include <GRUInferenceMethods/IEParams.hpp>
#include <IIRGRUInfo.hpp>
#include <IIRGRUUtils.hpp>
#include <IOStreamHandler.hpp>
#include <Player.hpp>
#include <Recorder.hpp>
#include <acquire_audio_stream.hpp>
#include <chrono>
#include <cxxopts.hpp>
static constexpr int32_t batch_size             = 1;
static constexpr int32_t dsp_audio_buffer_size  = 256;
static constexpr int32_t algo_audio_buffer_size = 256;
static constexpr int32_t input_size             = 2;
static constexpr int32_t hidden_size            = 128;
static constexpr int32_t num_layers             = 2;

class App {
   public:
    template <typename IEParams>
    App(const cxxopts::ParseResult&  args,
        const GeneralInferenceParams gparams,
        const IEParams&              ieparams,
        std::atomic<bool>&           running) :
        m_running{running},
        m_audio_buffer{4096},
        m_stream_handler{},
        m_recorder{m_stream_handler.get_in_sr(), 1, m_audio_buffer},
        m_player{m_gru,
                 gparams,
                 ieparams,
                 m_stream_handler.get_out_sr(),
                 1,
                 m_audio_buffer,
                 args["cpu_only"].as<bool>()},
        m_run_duration{-1} {
        parse_options(args);

        m_stream_handler.m_in_builder.setDataCallback(&m_recorder)
            ->setSampleRate(m_stream_handler.get_in_sr())
            ->setFramesPerCallback(dsp_audio_buffer_size);

        m_stream_handler.m_out_builder.setDataCallback(&m_player)
            ->setSampleRate(m_stream_handler.get_out_sr())
            ->setFramesPerCallback(dsp_audio_buffer_size);

        m_stream_handler.create_streams(dsp_audio_buffer_size);
    }
    ~App();  // does the dumping job

    void run();

   private:
    void parse_options(const cxxopts::ParseResult&);

   private:
    std::atomic<bool>& m_running;

    audio_buffer    m_audio_buffer;
    IOStreamHandler m_stream_handler;
    IIRGRUInfo<batch_size,
               algo_audio_buffer_size,
               input_size,
               hidden_size,
               num_layers>
        m_gru;

    Recorder                m_recorder;
    Player<decltype(m_gru)> m_player;
    int                     m_run_duration;
};