#pragma once

#include <ModelInferenceMethods/GRUInferenceMethods/IIRGRUUtils.hpp>
#include <ModelInferenceMethods/GeneralInferenceParams.hpp>
#include <boost/pfr.hpp>
#include <cxxopts.hpp>
#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <string>

auto get_options() {
    cxxopts::Options options{"filtered",
                             "Audio passing through filter program"};

    options.add_options()("h,help", "Print usage")(
        "m,model_filename",
        "File containing the model to load (expected .onnx file)",
        cxxopts::value<std::string>()->default_value("./lowpass_rnn.onnx"))(
        "t,model_type",
        "Type of the loaded model (supported : GRU, ConvTasNet)",
        cxxopts::value<std::string>()->default_value("ConvTasNet"))(
        "f,fc",
        "Cutoff frequency (Hz) - Needed for GRU model",
        cxxopts::value<int32_t>()->default_value("150"))(
        "p,profiling",
        "Profiling mode : get information about session perfomance (boolean)",
        cxxopts::value<bool>()->default_value("false"))(
        "P,profiling_file",
        "Profiling data file: filename to which to write the profiling data - "
        "should  be .npy extension (string)",
        cxxopts::value<std::string>()->default_value("latency.npy"))(
        "r,run_duration",
        "Run duration (seconds): indicate of much time to run the program (if "
        "not specified, the program runs until stopped with Ctrl+C)",
        cxxopts::value<int>())(
        "d,debug_mode_on",
        "Debug mode : get session input and output signals (boolean)",
        cxxopts::value<bool>()->default_value("false"))(
        "i,inference_engine",
        "Inference engine (IE) choice. Availble IEs are : "
        "Ort, Anira",
        cxxopts::value<std::string>()->default_value("Ort"))(
        "o,options",
        "Inference Engine options (json string) : \n"
        "Ort -> {\"EP_name\": string, \"EP_options\" : null|dict, "
        "\"config_entries\": null|dict }\n"
        "Anira -> {\"backend\": ONNX, \"model_latency\": float }\n",
        cxxopts::value<std::string>()->default_value(
            R"({"EP_name": "XNNPACK" })"))(
        "w,warm_up_buffers",
        "Amount of warm-up buffers (unsigned int)",
        cxxopts::value<size_t>()->default_value("0"))(
        "s,dsp_sample_rate",
        "DSP sample rate (unsigned int)",
        cxxopts::value<int>()->default_value("48000"))(
        "b,buffer_size",
        "Buffer size (unsigned int)",
        cxxopts::value<int>()->default_value("252"))(
        "ort_json_ep_options",
        "Ort specific: filename of json file containing EP options "
        "(std::string)",
        cxxopts::value<std::string>()->default_value(""))(
        "ort_json_config_entries",
        "Ort specific: filename of json file containing Ort config entries "
        "(std::string)",
        cxxopts::value<std::string>()->default_value(""));

    return options;
}

template <typename IEParams>
void fill_ie_params_from_args(IEParams& params, const std::string args) {
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

void fill_gparams_from_args(GeneralInferenceParams&     params,
                            const cxxopts::ParseResult& args) {
    boost::pfr::for_each_field(params, [&](auto& field, auto idx) {
        const auto field_name =
            std::string(boost::pfr::get_name<idx, GeneralInferenceParams>());
        if (args.contains(field_name)) {
            field =
                args[field_name]
                    .template as<std::remove_reference_t<decltype(field)>>();
        }
    });

    if (args["t"].as<std::string>() == "GRU") {
        params.Fc_normed =
            normalize_frequency((float)args["fc"].as<int32_t>(), 48000.f);
    }

    auto chosen_engine = magic_enum::enum_cast<SupportedInferenceEngines>(
        args["inference_engine"].as<std::string>());
    if (chosen_engine.has_value()) {
        params.chosen_engine = chosen_engine.value();
    }
}