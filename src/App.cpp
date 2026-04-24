#include <App.hpp>
#include <IIRGRUUtils.hpp>

App::~App() {
    m_recorder.dump("input.npy");

    if (m_player.debug) m_player.dump_debug("output.npy");
    if (m_player.profiling) m_player.dump_profiling("latency.npy");

    printf("Done.\n");
}

void App::run() {
    m_stream_handler.start_streams();
    printf("Audio passing through ...\n");

    {
        using namespace std::chrono;
        auto last_timestamp = steady_clock::now();
        int  delta          = 0;
        while (m_running) {
            delta = duration_cast<seconds>(steady_clock::now() - last_timestamp)
                        .count();
            if (m_run_duration > 0 && delta > m_run_duration) m_running = false;
            std::this_thread::sleep_for(150ms);
        }
    }
    m_stream_handler.stop_streams();
}

void App::parse_options(const cxxopts::ParseResult& args) {
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

    bool cpu_only = false;
    if (args.count("cpu_only") > 0) {
        cpu_only = args["cpu_only"].as<int>();
    }
    printf("CPU only : %s \n", cpu_only ? "yes" : "no");

    m_player.debug     = debug;
    m_player.profiling = profiling;
}