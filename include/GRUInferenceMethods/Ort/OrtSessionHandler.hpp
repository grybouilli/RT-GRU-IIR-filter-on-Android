#pragma once

#include <cpu_provider_factory.h>
#include <onnxruntime_cxx_api.h>

#include <iostream>
#include <magic_enum/magic_enum.hpp>
#include <string>
#include <vector>

enum class SupportedEPs {
    CPUExecutionProvider,
    QNNExecutionProvider,
    XnnpackExecutionProvider,
    WebGpuExecutionProvider,
    NnapiExecutionProvider
};

class OrtSessionHandler {
   public:
    OrtSessionHandler(const std::string model_filename,
                      const std::string ep_name,
                      const bool        debug = false) :
        m_env(debug ? ORT_LOGGING_LEVEL_VERBOSE : ORT_LOGGING_LEVEL_FATAL,
              "lowpass_rnn\n") {
        // Lists all providers compiled into your ORT build
        std::vector<std::string> providers = Ort::GetAvailableProviders();

        for (const auto& p : providers) {
            std::cout << "  - " << p << std::endl;
        }

        try {
            m_session_options.AppendExecutionProvider(ep_name);
            std::cout << ep_name << " registered" << std::endl;
        } catch (const Ort::Exception& e) {
            std::cout << "Exception caught : " << e.what() << std::endl;
            m_session_options.AppendExecutionProvider("CPUExecutionProvider");
            std::cout << "Falling back to "
                      << magic_enum::enum_name(
                             SupportedEPs::CPUExecutionProvider)
                      << std::endl;
        }

        m_session_options.SetIntraOpNumThreads(1);
        m_session_options.SetGraphOptimizationLevel(
            GraphOptimizationLevel::ORT_ENABLE_ALL);
        m_session_options.SetLogSeverityLevel(0);
        m_session_options.SetLogId("ort_session");

        m_session =
            Ort::Session(m_env, model_filename.c_str(), m_session_options);
    }

    Ort::Session& session() { return m_session.value(); }
    Ort::Env&     env() { return m_env; }

   private:
    Ort::Env                    m_env;
    Ort::SessionOptions         m_session_options;
    std::optional<Ort::Session> m_session;
};