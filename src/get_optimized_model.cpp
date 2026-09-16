/**
 * @file get_optimized_model.cpp
 * @author Nicolas Gry (nicolas.gry@madic.com)
 * @brief
 * @version 0.1
 * @date 2026-09-07
 *
 * @copyright Copyright (c) 2026
 *
 * @details
 * This program generates a compiled model. The generated model filename will
 * be: <ep_option_basename>_<model_filename>.onnx and will be put either in the
 * current working directory or in the directory passed with the flag --ctx_dest
 * <dir>
 *
 * So far, this has been mainly used to export models optimized for the HTP
 * with ONNX's QNN EP on Samsung S23.
 */
#include <sndfile.h>

#include <ModelInferenceMethods/ConvTasNetMethods/Ort/OrtConvTasNetInference.hpp>
#include <ModelInferenceMethods/GeneralInferenceParams.hpp>
#include <ModelInferenceMethods/IEParams.hpp>
#include <ModelInferenceMethods/OrtUtils/OrtSessionHandler.hpp>
#include <ModelInferenceMethods/OrtUtils/OrtTensorBuffer.hpp>
#include <nlohmann/json.hpp>
#include <npy.hpp>
#include <parsing_utils.hpp>
#include <random>
#include <utils.hpp>

using json = nlohmann::json;

constexpr size_t        upto           = 20;
static constexpr size_t in_shape_size  = 2;
static constexpr size_t out_shape_size = 3;
#define IN_SHAPE  {1, 24000}
#define OUT_SHAPE {1, 512, 3002}

int main(int argc, char** argv) {
    GeneralInferenceParams            gparams;
    OrtParams                         ort_params;
    ConvTasNetInfo<1, 24000, 2, 8000> model;

    auto options = get_options();
    options.add_options()("ctx_dest",
                          "Directory to which to save the compiled model",
                          cxxopts::value<std::string>()->default_value(""));
    auto args = options.parse(argc, argv);

    fill_gparams_from_args(gparams, args);
    fill_ie_params_from_args(ort_params, args["options"].as<std::string>());

    // Find loaded model filename and folder
    const std::string current_model_folder = get_folder(gparams.model_filename);
    const std::string current_model_basename =
        get_file_basename(gparams.model_filename);
    std::string compiled_model_filename = current_model_basename;
    std::string ep_opt_json_file        = "";
    // add ep options if provided by user
    if (const auto ep_options_json =
            args["ort_json_ep_options"].as<std::string>();
        ep_options_json != "") {
        std::ifstream f(ep_options_json);
        json          ep_options = json::parse(f,
                                               /* callback */ nullptr,
                                               /* allow exceptions */ true,
                                               /* ignore_comments */ true);

        ort_params.EP_options = ep_options;

        const std::string base_filename = get_file_basename(ep_options_json);
        const std::string file_without_extension =
            get_file_basename_no_ext(base_filename);

        compiled_model_filename =
            file_without_extension + "_" + current_model_basename;
        ep_opt_json_file = file_without_extension;
    }

    // Handle destination output folder
    if (const auto dest_ctx_folder = args["ctx_dest"].as<std::string>();
        dest_ctx_folder != "") {
        compiled_model_filename =
            dest_ctx_folder + "/" + compiled_model_filename;
    } else {
        compiled_model_filename =
            current_model_folder + "/" + compiled_model_filename;
    }

    // Set-up profiling if enabled
    if (ort_params.EP_options.contains("profiling_level")) {
        ort_params.EP_options["profiling_file_path"] =
            "logs/" + ep_opt_json_file + "_" + current_model_basename + ".csv";
        std::cout << "Profiling data will be put at "
                  << ort_params.EP_options["profiling_file_path"] << std::endl;
    }

    // enables ep context when ort_json_config_entries is not provided
    if (const auto config_entries_json =
            args["ort_json_config_entries"].as<std::string>();
        config_entries_json != "") {
        std::ifstream f(config_entries_json);
        json          config_entries = json::parse(f);

        ort_params.config_entries = config_entries;
    } else {
        // add entries to export .ctx model
        ort_params.config_entries[kOrtSessionOptionEpContextEnable]    = "1";
        ort_params.config_entries[kOrtSessionOptionEpContextEmbedMode] = "1";
        ort_params.config_entries[kOrtSessionOptionEpContextFilePath] =
            compiled_model_filename;
    }

    OrtConvTasNetInference interface{model, gparams, ort_params};
    return 0;
}