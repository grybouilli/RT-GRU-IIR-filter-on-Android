#pragma once

#include <string>
enum class SupportedInferenceEngines { Ort };

struct GeneralInferenceParams {
    std::string               model_filename;
    bool                      debug_mode_on;
    SupportedInferenceEngines chosen_engine;
};