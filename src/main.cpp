#include <App.hpp>
#include <ModelTypes.hpp>
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

    if (args["model_type"].as<std::string>() ==
        magic_enum::enum_name(ModelType::GRU)) {
        static constexpr int32_t batch_size             = 1;
        static constexpr int32_t algo_audio_buffer_size = 256;
        static constexpr int32_t input_size             = 2;
        static constexpr int32_t hidden_size            = 128;
        static constexpr int32_t num_layers             = 2;
        IIRGRUInfo<batch_size,
                   algo_audio_buffer_size,
                   input_size,
                   hidden_size,
                   num_layers>
            gru;
        App app(gru, args, gparams, params, run);
        app.run();
    }

    return EXIT_SUCCESS;
}

int main_body(int argc, char** argv) {
    cxxopts::Options options{"filtered",
                             "Audio passing through filter program"};

    options.add_options()("h,help", "Print usage")(
        "m,model",
        "File containing the model to load (expected .onnx file)",
        cxxopts::value<std::string>()->default_value("./lowpass_rnn.onnx"))(
        "t,model_type",
        "Type of the loaded model (supported : GRU)",
        cxxopts::value<std::string>()->default_value(
            "GRU"))("f,fc", "Cutoff frequency (Hz)", cxxopts::value<int32_t>())(
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
    return 0;
}

#ifndef APP_AS_APK
#pragma message("Compiling for native run")
int main(int argc, char** argv) {
    signal(SIGINT, sigint_handler);
    return main_body(argc, argv);
}
#else
#include <android/log.h>
#include <jni.h>
#include <pthread.h>
#include <unistd.h>

#define TAG "WrapperApp"

static void* stdoutToLogcat(void* arg) {
    int     fd = (int)(intptr_t)arg;
    char    buf[1024];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        __android_log_print(ANDROID_LOG_INFO, TAG, "%s", buf);
    }
    return nullptr;
}

static void redirectStdoutToLogcat() {
    int pipefd[2];
    pipe(pipefd);

    // Replace stdout and stderr with the write end of the pipe
    dup2(pipefd[1], STDOUT_FILENO);
    dup2(pipefd[1], STDERR_FILENO);
    close(pipefd[1]);

    // Spawn a thread to read the read end and forward to logcat
    pthread_t thread;
    pthread_create(&thread,
                   nullptr,
                   stdoutToLogcat,
                   (void*)(intptr_t)pipefd[0]);
    pthread_detach(thread);
}

#pragma message("Compiling for embedding in APK app")
std::vector<std::string> parseArgs(const std::string& input) {
    std::vector<std::string> tokens;
    std::string              current;
    char                     quoteChar = 0;
    bool                     inQuote   = false;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];

        if (std::isspace(c) && !current.empty()) {
            tokens.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);  // last token
    }

    return tokens;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_wrapperapp_MainActivity_runFiltered(JNIEnv* env,
                                                     jobject,
                                                     jstring jargs) {
    redirectStdoutToLogcat();  // call once before anything else
    const char* argsStr = env->GetStringUTFChars(jargs, nullptr);
    std::string argsStdStr(argsStr);
    env->ReleaseStringUTFChars(jargs, argsStr);

    std::vector<std::string> tokens = parseArgs(argsStdStr);

    std::vector<char*> argv;
    argv.push_back(const_cast<char*>("filtered"));  // argv[0]
    for (auto& t : tokens) {
        argv.push_back(t.data());
    }
    int argc = argv.size();

    __android_log_print(ANDROID_LOG_INFO,
                        TAG,
                        "Calling main_body() with argc=%d",
                        argc);
    for (int i = 0; i < argc; i++) {
        __android_log_print(ANDROID_LOG_INFO,
                            TAG,
                            "  argv[%d] = %s",
                            i,
                            argv[i]);
    }

    main_body(argc, argv.data());
}
#endif