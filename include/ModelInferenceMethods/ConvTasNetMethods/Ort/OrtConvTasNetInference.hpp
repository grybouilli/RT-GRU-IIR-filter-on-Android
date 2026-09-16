#pragma once

#include <AudioParams.hpp>
#include <ModelInferenceMethods/ConvTasNetMethods/ConvTasNetInfo.hpp>
#include <ModelInferenceMethods/GeneralInferenceParams.hpp>
#include <ModelInferenceMethods/ModelInferenceMethodBase.hpp>
#include <ModelInferenceMethods/OrtUtils/OrtSessionHandler.hpp>
#include <ModelInferenceMethods/OrtUtils/OrtTensorBuffer.hpp>
#include <Resampler.hpp>
#include <array>

template <IsConvTasNetInfo ConvTasNet>
class OrtConvTasNetInference final
    : public ModelInferenceMethodBase<ConvTasNet> {
   public:
    OrtConvTasNetInference(const ConvTasNet&            convtasnet,
                           const GeneralInferenceParams gparams,
                           const OrtParams&             ieparams) :
        ModelInferenceMethodBase<ConvTasNet>(convtasnet, gparams, ieparams),
        m_session_handler{gparams.model_filename,
                          ieparams.EP_name,
                          gparams.debug_mode_on,
                          ieparams.EP_options,
                          ieparams.config_entries},
        m_memory_info{
            Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)},
        m_binding{m_session_handler.session()},
        m_x_data{m_memory_info,
                 {convtasnet.batch_size(), convtasnet.buffer_size()}},
        m_output{m_memory_info,
                 {convtasnet.batch_size(),
                  convtasnet.output_channels(),
                  convtasnet.buffer_size()}},
        m_downsampler{(double)convtasnet.sample_rate() /
                      gparams.dsp_sample_rate},
        m_upsampler{(double)gparams.dsp_sample_rate / convtasnet.sample_rate()},
        m_selected_out_channel{0} {}

    bool run(float* audio, const size_t num_samples) override {
        const auto expected_ds_size = prepare_input(audio, num_samples);

        m_binding.ClearBoundInputs();
        m_binding.ClearBoundOutputs();
        m_binding.BindInput("input", m_x_data.tensor);
        m_binding.BindOutput("output", m_output.tensor);

        m_session_handler.session().Run(Ort::RunOptions{nullptr}, m_binding);

        transfer_output(audio, num_samples, expected_ds_size);

        return true;  // TODO: return the amount of treated samples
    }

    /**
     * @brief Select which output channel to output when using run
     *
     * @param channel an integer that is either 0 or 1
     */
    void select_output_channel(const size_t channel) {
        m_selected_out_channel = channel % 2;
    }

    void toggle_resampling(const bool activate) {
        m_resample_activated = activate;
    }

   private:
    /**
     * @brief Transfers \p audio in the input buffer, after resampling it if
     * needed.
     *
     * @param audio
     * @param num_samples
     * @return * size_t The amount of new samples entering the network
     */
    size_t prepare_input(float* audio, const size_t num_samples) {
        static const size_t B = static_cast<size_t>(ConvTasNet::buffer_size());
        const auto          samples = std::min(num_samples, B);
        const size_t        expected_ds_size =
            m_resample_activated ? (int)(m_downsampler.get_ratio() * samples)
                                 : samples;
        // x_data contains a buffer the size of the model's input.
        // it will thus contain older, already seen buffers
        // here we shift left the content of the buffer, discarding the oldest
        // buffer
        std::shift_left(m_x_data.buffer_memory.begin(),
                        m_x_data.buffer_memory.end(),
                        expected_ds_size);  // discard the oldest buffer

        float* latest_input_buffer = m_x_data.buffer_memory.data() +
                                     m_x_data.buffer_memory.size() -
                                     expected_ds_size;
        int    ds_frames;
        // we copy the current buffer in the last slot of x_data either when
        // resampling, or directly if resampling is deactivated
        if (m_resample_activated) {
            ds_frames = m_downsampler.resample(audio,
                                               num_samples,
                                               latest_input_buffer,
                                               expected_ds_size);
        } else {
            ds_frames = expected_ds_size;
            std::memcpy(latest_input_buffer,
                        audio,
                        expected_ds_size * sizeof(float));
        }

        return ds_frames;
    }

    void transfer_output(float*       audio,
                         const size_t samples,
                         const size_t ds_frames) {
        static const size_t B = static_cast<size_t>(ConvTasNet::buffer_size());
        // -- Post inference process
        // Locate the last buffer in the output, which like the input, contains
        // old data
        const auto offset = (m_selected_out_channel + 1) * B - ds_frames;
        if (m_resample_activated) {
            std::vector<float> upsampled_voices(samples);
            auto               gen_frames =
                m_upsampler.resample(m_output.buffer_memory.data() + offset,
                                     ds_frames,
                                     upsampled_voices.data(),
                                     upsampled_voices.size());
            const auto upsampled_frames = std::min((int)samples, gen_frames);

            std::memset(
                audio,
                0,
                samples *
                    sizeof(
                        float));  // DEBUG : check if output is not just input
            for (auto sample = 0; sample < upsampled_frames; ++sample) {
                *(audio + sample) = upsampled_voices[sample];
            }
        } else {
            std::memcpy(audio,
                        m_output.buffer_memory.data() + offset,
                        sizeof(float) * ds_frames);
        }
    }
    OrtSessionHandler m_session_handler;

    Ort::MemoryInfo m_memory_info;
    Ort::IoBinding  m_binding;

    OrtTensorBuffer<float, 2> m_x_data;
    OrtTensorBuffer<float, 3> m_output;

    Resampler<1> m_downsampler;
    Resampler<2> m_upsampler;
    bool         m_resample_activated;

    size_t m_selected_out_channel;
};