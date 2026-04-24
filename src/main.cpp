#include <App.hpp>

std::atomic<bool> run = true;

void sigint_handler(int arg) { run = false; }

int main(int argc, char** argv) {
    signal(SIGINT, sigint_handler);
    cxxopts::Options options{"FilterProgram",
                             "Audio passing through filter program"};

    options.add_options()("h,help", "Print usage")(
        "m,model",
        "File containing the model to load (expected .onnx file)",
        cxxopts::value<std::string>()->default_value("./lowpass_rnn.onnx"))(
        "f,fc",
        "Cutoff frequency (Hz)",
        cxxopts::value<int32_t>())(
        "p,profiling",
        "Profiling mode : get information about session perfomance (boolean)",
        cxxopts::value<bool>()->default_value("false"))(
        "r,run_duration",
        "Run duration (seconds): indicate of much time to run the program (if "
        "not specified, the program runs until stopped with Ctrl+C)",
        cxxopts::value<int>())(
        "d,debug",
        "Debug mode : get session input and output signals (boolean)",
        cxxopts::value<bool>()->default_value("false"))(
        "i,inference_engine",
        "Inference engine (IE) choice. Availble IEs are : "
        "Ort",
        cxxopts::value<std::string>()->default_value("Ort"))(
        "e,ep",
        "Execution Provider selection. Availble EPs are : "
        "NnapiExecutionProvider, WebGpuExecutionProvider, "
        "XnnpackExecutionProvider, CPUExecutionProvider",
        cxxopts::value<std::string>()->default_value(
            "XnnpackExecutionProvider"))(
        "c,cpu_only",
        "CPU only mode : NNAPI will not try to run inference on GPU/NPU "
        "(boolean)",
        cxxopts::value<bool>()->default_value("false"));

    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }
    GeneralInferenceParams gparams;

    gparams.model_filename = args["model"].as<std::string>();
    gparams.debug_mode_on  = args["debug"].as<bool>();

    auto chosen_engine = args["inference_engine"].as<std::string>();

    if (chosen_engine == "Ort") {
        OrtParams ort_params;
        ort_params.EP_name = args["ep"].as<std::string>();
        ort_params.Fc_normed =
            normalize_frequency((float)args["fc"].as<int32_t>(), 48000.f);

        std::cout << std::format("normed cut off freq = {}",
                                 ort_params.Fc_normed)
                  << std::endl;
        gparams.chosen_engine = SupportedInferenceEngines::Ort;
        App app(args, gparams, ort_params, run);

        app.run();
        return EXIT_SUCCESS;
    }
}
