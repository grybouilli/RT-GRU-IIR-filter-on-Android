#pragma once

#include <onnxruntime_cxx_api.h>
#include <signal.h>

#include <AudioParams.hpp>
#include <IIRGRUInfo.hpp>
#include <IIRGRUUtils.hpp>
#include <IOStreamHandler.hpp>
#include <ModelInferenceMethods/GeneralInferenceParams.hpp>
#include <ModelInferenceMethods/IEParams.hpp>
#include <Player.hpp>
#include <Recorder.hpp>
#include <acquire_audio_stream.hpp>
#include <chrono>
#include <cxxopts.hpp>

template <typename ModelInfo>
class App {
   public:
    template <typename IEParams>
    App(const ModelInfo              model,
        const cxxopts::ParseResult&  args,
        const GeneralInferenceParams gparams,
        const IEParams&              ieparams,
        std::atomic<bool>&           running) :
        m_running{running},
        m_audio_buffer{4096},
        m_stream_handler{},
        m_recorder{m_stream_handler.get_in_sr(), 1, m_audio_buffer},
        m_model{model},
        m_player{m_model,
                 gparams,
                 ieparams,
                 m_stream_handler.get_out_sr(),
                 1,
                 m_audio_buffer},
        m_run_duration{-1},
        m_profiling_output_file{gparams.profiling_file} {
        parse_options(args);

        constexpr int32_t dsp_audio_buffer_size = 256;
        m_stream_handler.m_in_builder.setDataCallback(&m_recorder)
            ->setSampleRate(m_stream_handler.get_in_sr())
            ->setFramesPerCallback(dsp_audio_buffer_size);

        m_stream_handler.m_out_builder.setDataCallback(&m_player)
            ->setSampleRate(m_stream_handler.get_out_sr())
            ->setFramesPerCallback(dsp_audio_buffer_size);

        m_stream_handler.create_streams(dsp_audio_buffer_size);
    }
    ~App() {
        m_recorder.dump("input.npy");

        if (m_player.debug) m_player.dump_debug_output("output.npy");
        if (m_player.debug) m_player.dump_debug_timestamps("timestamps.npy");
        if (m_player.profiling)
            m_player.dump_profiling(m_profiling_output_file);

        printf("Done.\n");
    }

    void run() {
        m_stream_handler.start_streams();
        printf("Audio passing through ...\n");

        {
            using namespace std::chrono;
            auto last_timestamp = steady_clock::now();
            int  delta          = 0;
            while (m_running) {
                delta =
                    duration_cast<seconds>(steady_clock::now() - last_timestamp)
                        .count();
                if (m_run_duration > 0 && delta > m_run_duration)
                    m_running = false;
                std::this_thread::sleep_for(150ms);
            }
        }
        m_stream_handler.stop_streams();
    }

   private:
    void parse_options(const cxxopts::ParseResult& args) {
        bool profiling = false;
        if (args.count("profiling") > 0) {
            profiling = args["profiling"].as<bool>();
        }
        printf("Profiling active : %s\n", profiling ? "yes" : "no");

        bool debug = false;
        if (args.count("debug") > 0) {
            debug = args["debug"].as<bool>();
        }
        printf("Debug active : %s\n", debug ? "yes" : "no");

        if (args.count("run_duration") > 0) {
            m_run_duration = args["run_duration"].as<int>();
            printf("Run duration : %d seconds\n", m_run_duration);
        }

        m_player.debug     = debug;
        m_player.profiling = profiling;
    }

   private:
    std::atomic<bool>& m_running;

    audio_buffer    m_audio_buffer;
    IOStreamHandler m_stream_handler;

    Recorder          m_recorder;
    ModelInfo         m_model;
    Player<ModelInfo> m_player;
    int               m_run_duration;
    std::string       m_profiling_output_file;
};