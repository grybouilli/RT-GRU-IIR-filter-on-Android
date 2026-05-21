#pragma once

#include <anira/anira.h>

class PrePostGRUProcessor : public anira::PrePostProcessor {
   public:
    // Inherit constructor from base class
    using anira::PrePostProcessor::PrePostProcessor;

    PrePostGRUProcessor(anira::InferenceConfig& inference_config,
                        const size_t            num_layers,
                        const size_t            hidden_size,
                        const float             normalized_fc) :
        PrePostProcessor(inference_config),
        m_normalized_cutoff_freq{normalized_fc},
        m_hidden_state(num_layers * hidden_size, 0.f) {}

    virtual void pre_process(std::vector<anira::RingBuffer>& input,
                             std::vector<anira::BufferF>&    output,
                             [[maybe_unused]] anira::InferenceBackend
                                 current_inference_backend) override {
        static const auto buffer_size =
            m_inference_config.get_tensor_input_shape()
                [0][1];  // input shape is { {1, buffer_size, 2}, {layer_num, 1,
                         // hidden_size} }
        static const auto num_layers =
            m_inference_config.get_tensor_input_shape()[1][0];
        static const auto hidden_size =
            m_inference_config.get_tensor_input_shape()[1][2];

        // interleave audio samples and normalized frequency
        for (auto sample = 0;
             sample < m_inference_config.get_preprocess_input_size()[0];
             ++sample) {
            output[0].set_sample(0, sample * 2, input[0].pop_sample(0));
            output[0].set_sample(0, sample * 2 + 1, m_normalized_cutoff_freq);
        }

        // hidden state copy
        static const size_t total = num_layers * hidden_size;
        std::memcpy(output[1].get_write_pointer(0),
                    m_hidden_state.data(),
                    total * sizeof(float));
    }

    virtual void post_process(std::vector<anira::BufferF>&    input,
                              std::vector<anira::RingBuffer>& output,
                              [[maybe_unused]] anira::InferenceBackend
                                  current_inference_backend) override {
        static const auto buffer_size =
            m_inference_config.get_tensor_input_shape()
                [0][1];  // input shape is { {1, buffer_size, 2}, {layer_num, 1,
                         // hidden_size} }
        static const auto num_layers =
            m_inference_config.get_tensor_input_shape()[1][0];
        static const auto hidden_size =
            m_inference_config.get_tensor_input_shape()[1][2];

        for (size_t i = 0; i < buffer_size; ++i) {
            output[0].push_sample(0, input[0].get_sample(0, i));
        }

        // hidden state copy
        static const size_t total = num_layers * hidden_size;
        std::memcpy(m_hidden_state.data(),
                    input[1].get_read_pointer(0),
                    total * sizeof(float));
    }

    float m_normalized_cutoff_freq;

   private:
    std::vector<float> m_hidden_state;
};