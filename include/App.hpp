#pragma once

#include <AudioParams.hpp>
#include <Recorder.hpp>
#include <Player.hpp>
#include <acquire_audio_stream.hpp>
#include <signal.h>
#include <IIRGRUInfo.hpp>
#include <IIRGRUUtils.hpp>
#include <onnxruntime_cxx_api.h>
#include <cxxopts.hpp>
#include <chrono>
#include <IOStreamHandler.hpp>

static constexpr int32_t batch_size = 1;
static constexpr int32_t dsp_audio_buffer_size = 256;
static constexpr int32_t algo_audio_buffer_size = 256;
static constexpr int32_t input_size = 2;
static constexpr int32_t hidden_size = 64;
static constexpr int32_t num_layers = 2;

class App
{
public:
    App(int argc, char ** argv, std::atomic<bool>& running);
    ~App(); // does the dumping job

    void run();

private:
    cxxopts::ParseResult parse_options(const int argc, char ** argv);

private:
    std::atomic<bool>&          m_running;

    cxxopts::Options            m_options;
    audio_buffer                m_audio_buffer;
    IOStreamHandler             m_stream_handler;
    IRRGRUInfo< batch_size, 
                algo_audio_buffer_size, 
                input_size, 
                hidden_size, 
                num_layers>     m_gru;

    Recorder                    m_recorder;
    Player<decltype(m_gru)>     m_player;
    int                         m_run_duration;
};