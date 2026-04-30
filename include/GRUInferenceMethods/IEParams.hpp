#pragma once

#include <string>
#include <unordered_map>

struct OrtParams {
    std::string EP_name;
};

struct AniraParams {
    std::string backend;
    float       model_latency;
};