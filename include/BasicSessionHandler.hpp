#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <onnxruntime_cxx_api.h>
#include <cpu_provider_factory.h>
#include <nnapi_provider_factory.h>

class BasicSessionHandler {
public:

    BasicSessionHandler(const std::string model_filename)
    : m_env(ORT_LOGGING_LEVEL_WARNING, "lowpass_rnn\n")
    {
        // Lists all providers compiled into your ORT build
        std::vector<std::string> providers = Ort::GetAvailableProviders();

        for (const auto& p : providers) {
            std::cout << "  - " << p << std::endl;
        }

        // Use NNAPI if possible
        uint32_t nnapi_flags = 0;
        OrtStatus* status =
            OrtSessionOptionsAppendExecutionProvider_Nnapi(m_session_options, nnapi_flags);

        if (status != nullptr) {
            // NNAPI EP not compiled in or not available on this device – fall back silently.
            std::string err = Ort::GetApi().GetErrorMessage(status);
            Ort::GetApi().ReleaseStatus(status);
            std::cout << "NNAPI EP unavailable (" << err << "), using ORT CPU." << std::endl;
        } else {
            std::cout << "NNAPI EP registered (GPU/NPU preferred)." << std::endl;
        }
        
        m_session_options.SetIntraOpNumThreads(1);
        m_session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        m_session = Ort::Session(m_env, model_filename.c_str(), m_session_options);
    }

    Ort::Session& session() { return m_session.value(); }
    
private:
    Ort::Env m_env;
    Ort::SessionOptions m_session_options;
    std::optional<Ort::Session> m_session;
};