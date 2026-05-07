#include <App.hpp>
#include <boost/pfr.hpp>
#include <nlohmann/json.hpp>

std::atomic<bool> run = true;

void sigint_handler(int arg) { run = false; }

template <typename IEParams>
void fill_params_from_args(IEParams& params, const std::string args) {
    using json = nlohmann::json;

    json params_json = json::parse(args);

    boost::pfr::for_each_field(params, [&](auto& field, auto idx) {
        constexpr auto field_name = boost::pfr::get_name<idx, IEParams>();
        if (params_json.contains(field_name)) {
            field =
                params_json[field_name]
                    .template get<std::remove_reference_t<decltype(field)>>();
        }
    });
}

template <typename IEParams>
int runApp(const cxxopts::ParseResult&     args,
           GeneralInferenceParams&         gparams,
           const SupportedInferenceEngines engine) {
    IEParams params;
    fill_params_from_args(params, args["options"].as<std::string>());

    gparams.chosen_engine = engine;
    App app(args, gparams, params, run);

    app.run();
    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    signal(SIGINT, sigint_handler);
    cxxopts::Options options{"filtered",
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
        "P,profiling_data",
        "Profiling data file: filename to which to write the profiling data - "
        "should  be .npy extension (string)",
        cxxopts::value<std::string>()->default_value("latency.npy"))(
        "r,run_duration",
        "Run duration (seconds): indicate of much time to run the program (if "
        "not specified, the program runs until stopped with Ctrl+C)",
        cxxopts::value<int>())(
        "d,debug",
        "Debug mode : get session input and output signals (boolean)",
        cxxopts::value<bool>()->default_value("false"))(
        "i,inference_engine",
        "Inference engine (IE) choice. Availble IEs are : "
        "Ort, Anira",
        cxxopts::value<std::string>()->default_value("Ort"))(
        "o,options",
        "Inference Engine options (json string) : \n"
        "Ort -> {\"EP_name\": string, \"EP_options\" : null|dict }\n"
        "Anira -> {\"backend\": ONNX, \"model_latency\": float }\n",
        cxxopts::value<std::string>()->default_value(
            R"({"EP_name": "XNNPACK" })"));

    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }
    GeneralInferenceParams gparams;

    gparams.model_filename = args["model"].as<std::string>();
    gparams.debug_mode_on  = args["debug"].as<bool>();
    gparams.Fc_normed =
        normalize_frequency((float)args["fc"].as<int32_t>(), 48000.f);
    gparams.profiling_file = args["profiling_data"].as<std::string>();

    std::cout << std::format("normed cut off freq = {}", gparams.Fc_normed)
              << std::endl;
    auto chosen_engine = args["inference_engine"].as<std::string>();

    constexpr std::array possible_engines = {"Ort", "Anira"};

    if (chosen_engine ==
        possible_engines[(size_t)SupportedInferenceEngines::Ort]) {
        return runApp<OrtParams>(args, gparams, SupportedInferenceEngines::Ort);
    }

    if (chosen_engine ==
        possible_engines[(size_t)SupportedInferenceEngines::Anira]) {
        return runApp<AniraParams>(args,
                                   gparams,
                                   SupportedInferenceEngines::Anira);
    }
}
