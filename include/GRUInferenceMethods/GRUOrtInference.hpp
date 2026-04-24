#pragma once

#include <GRUInferenceMethods/GRUInferenceMethodBase.hpp>
#include <GRUInferenceMethods/GeneralInferenceParams.hpp>
#include <GRUInferenceMethods/OrtSessionHandler.hpp>
#include <GRUInferenceMethods/OrtTensorBuffer.hpp>

template <IsIIRGRUInfo IIRGRU>
class GRUOrtInference final : public GRUInferenceMethodBase<IIRGRU> {
   public:
    GRUOrtInference(const IIRGRU&                gru,
                    const GeneralInferenceParams gparams,
                    const OrtParams&             ieparams) :
        GRUInferenceMethodBase<IIRGRU>(gru, gparams, ieparams),
        m_session_handler{gparams.model_filename,
                          ieparams.EP_name,
                          gparams.debug_mode_on},
        m_fc_normed{ieparams.Fc_normed},
        m_memory_info{
            Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)},
        m_binding{m_session_handler.session()},
        m_x_data{m_memory_info,
                 {gru.batch_size(), gru.buffer_size(), gru.input_size()}},
        m_hidden{
            {OrtTensorBuffer(
                 m_memory_info,
                 {gru.num_layers(), gru.batch_size(), gru.hidden_size()}),
             OrtTensorBuffer(
                 m_memory_info,
                 {gru.num_layers(), gru.batch_size(), gru.hidden_size()})}},
        m_output{m_memory_info, {gru.batch_size(), gru.buffer_size(), 1}} {
        printf("normed frequency is %f\n", m_fc_normed);

        const auto Ba = gru.batch_size();
        const auto I  = gru.input_size();
        const auto B  = gru.buffer_size();
        const auto H  = gru.hidden_size();
        const auto L  = gru.num_layers();

        // Pre-fill the fc channel (odd indices)
        for (size_t i = 0; i < static_cast<size_t>(B); ++i) {
            m_x_data.buffer_memory[i * 2]     = 0.f;
            m_x_data.buffer_memory[i * 2 + 1] = m_fc_normed;
        }

        m_hidden_ping = 0;  // index of current hidden_in buffer
    }

    bool run(const float* audio_in, float* audio_out) override {
        static const size_t B = static_cast<size_t>(m_gru.buffer_size());

        // Fill audio channel of x with new input values
        for (size_t i = 0; i < B; ++i)
            m_x_data.buffer_memory[i * 2] = audio_in[i];

        // Swap hidden ping-pong:
        const int pong = 1 - m_hidden_ping;
        m_binding.ClearBoundInputs();

        m_binding.BindInput("x", m_x_data.tensor);
        m_binding.BindInput("hidden_in", m_hidden[m_hidden_ping].tensor);

        m_binding.ClearBoundOutputs();
        m_binding.BindOutput("output", m_output.tensor);
        m_binding.BindOutput("hidden_out", m_hidden[pong].tensor);

        m_session_handler.session().Run(Ort::RunOptions{nullptr}, m_binding);

        m_hidden_ping = pong;

        std::memcpy(audio_out,
                    m_output.buffer_memory.data(),
                    B * sizeof(float));

        return true;
    }

    void set_normed_fc(const float nfc) {
        m_fc_normed  = nfc;
        const auto B = m_gru.buffer_size();
        for (size_t i = 0; i < static_cast<size_t>(B); ++i) {
            m_x_data.buffer_memory[i * 2 + 1] = nfc;  // fc    — constant
        }
    }

   private:
    OrtSessionHandler m_session_handler;
    IIRGRU            m_gru;
    float             m_fc_normed;

    Ort::MemoryInfo m_memory_info;
    Ort::IoBinding  m_binding;

    size_t                         m_hidden_ping;
    OrtTensorBuffer                m_x_data;
    std::array<OrtTensorBuffer, 2> m_hidden;
    OrtTensorBuffer                m_output;
};