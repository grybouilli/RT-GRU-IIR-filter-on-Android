#include <App.hpp>
#include <IIRGRUUtils.hpp>

App::App(int argc, char ** argv, std::atomic<bool>& running)
: m_running {running}
, m_options {("FilterProgram", "Audio passing through filter program")}
, m_audio_buffer {4096}
, m_stream_handler{}
, m_recorder{m_stream_handler.get_in_sr(), 1, m_audio_buffer}
, m_player {m_gru, 0, m_stream_handler.get_out_sr(), 1, m_audio_buffer}
, m_run_duration {-1}
{
    m_options.add_options()
    ("f,fc",        "Cutoff frequency (Hz)", cxxopts::value<int32_t>())
    ("p,profiling", "Profiling mode : get information about session perfomance", cxxopts::value<bool>()->default_value("false"))
    ("r,run_duration", "Run duration (seconds): indicate of much time to run the program (if not specified, the program runs until stopped with Ctrl+C)", cxxopts::value<int>())
    ("d,debug", "Debug mode : get session input and output signals", cxxopts::value<bool>()->default_value("false"))
    ;

    auto args = parse_options(argc, argv);
    m_player.set_normed_fc(
        normalize_frequency( (float)args["fc"].as<int32_t>(), (float)m_stream_handler.get_out_sr() )
    );

    m_stream_handler
        .m_in_builder.setDataCallback(&m_recorder)
        ->setSampleRate(m_stream_handler.get_in_sr())
        ->setFramesPerCallback(dsp_audio_buffer_size);

    m_stream_handler
        .m_out_builder.setDataCallback(&m_player)
        ->setSampleRate(m_stream_handler.get_out_sr())
        ->setFramesPerCallback(dsp_audio_buffer_size);

    m_stream_handler.create_streams(dsp_audio_buffer_size);
}

App::~App()
{
    m_recorder.dump("input.npy");
    
    if(m_player.debug)
        m_player.dump_debug("output.npy");
    if(m_player.profiling)
        m_player.dump_profiling("latency.npy");
        
        printf("Done.\n");
}

void App::run()
{
    m_stream_handler.start_streams();
    printf("Audio passing through ...\n");

    {
        using namespace std::chrono;
        auto last_timestamp = steady_clock::now();
        int delta = 0; 
        while (m_running)
        {
            delta = duration_cast<seconds>(steady_clock::now() - last_timestamp).count();
            if(m_run_duration > 0 && delta > m_run_duration)
                m_running = false;
            std::this_thread::sleep_for(150ms);
        }
    }
    m_stream_handler.stop_streams();
}

cxxopts::ParseResult App::parse_options(const int argc, char ** argv)
{
    auto args = m_options.parse(argc, argv);
    bool profiling = false;
    if(args.count("profiling") > 0)
    {
        profiling = args["profiling"].as<bool>();
        printf("Profiling active : %s\n", profiling ? "yes" : "no");
    }

    bool debug = false;
    if(args.count("debug") > 0)
    {
        debug = args["debug"].as<bool>();
        printf("Debug active : %s\n", debug ? "yes" : "no");
    }

    if(args.count("run_duration") > 0)
    {
        m_run_duration = args["run_duration"].as<int>();
        printf("Run duration : %d seconds\n", m_run_duration);
    }

    m_player.debug = debug;
    m_player.profiling = profiling;

    return args;
}