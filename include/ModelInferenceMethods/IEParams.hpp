#pragma once

#include <string>
#include <unordered_map>

struct OrtParams {
    std::string                                  EP_name;
    std::unordered_map<std::string, std::string> EP_options;
};

struct AniraParams {
    std::string backend;
    float       model_latency;
};