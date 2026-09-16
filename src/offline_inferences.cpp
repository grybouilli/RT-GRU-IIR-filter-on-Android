#include <sndfile.h>
#define OFFLINE_INFERENCES 1

#include <ModelInferenceMethods/ConvTasNetMethods/ConvTasNetInfo.hpp>
#include <ModelInferenceMethods/ConvTasNetMethods/Ort/OrtConvTasNetInference.hpp>
#include <ModelInferenceMethods/GeneralInferenceParams.hpp>
#include <SoundFileReader.hpp>
#include <SoundFileWriter.hpp>
#include <npy.hpp>
#include <parsing_utils.hpp>
#include <utils.hpp>

#ifndef INPUT_SAMPLE_COUNT
#define INPUT_SAMPLE_COUNT 16000
#endif

int main(int argc, char** argv) {
    GeneralInferenceParams                         gparams;
    OrtParams                                      ort_params;
    ConvTasNetInfo<1, INPUT_SAMPLE_COUNT, 2, 8000> model;

    auto options = get_options();
    options.add_options()("file",
                          "Input file for offline inference",
                          cxxopts::value<std::string>());

    options.add_options()("aux_destination_folder",
                          "Folder that will contain auxiliary generated files "
                          "(audio output and latency data)",
                          cxxopts::value<std::string>());

    auto args = options.parse(argc, argv);

    fill_gparams_from_args(gparams, args);
    std::cout << "Test" << std::endl;
    fill_ie_params_from_args(ort_params, args["options"].as<std::string>());
    std::cout << "DSP sample rate = " << gparams.dsp_sample_rate << std::endl;

    constexpr size_t  output_sources_count = 2;
    const std::string aux_out_folder =
        args["aux_destination_folder"].as<std::string>();
    const std::string input_basename =
        get_file_basename_no_ext(args["file"].as<std::string>());
    for (auto out_channel = 0; out_channel < output_sources_count;
         ++out_channel) {
        OrtConvTasNetInference interface{model, gparams, ort_params};
        interface.select_output_channel(out_channel);

        std::cout << "Opening file " << args["file"].as<std::string>()
                  << std::endl;
        // read audio file
        SoundFileReader<float, SF_FORMAT_WAV> input_audio_file{
            args["file"].as<std::string>(),
            gparams.dsp_sample_rate,
            1};

        // variables for output file
        const std::string outfilename =
            std::format("{}/outputs/{}_output_channel_{}.wav",
                        aux_out_folder,
                        input_basename,
                        out_channel);
        SoundFileWriter<float, SF_FORMAT_WAV> output_audio_file{
            outfilename,
            gparams.dsp_sample_rate,
            1};

        std::vector<float> output_audio{};
        output_audio.reserve(input_audio_file.frames() *
                             input_audio_file.channels());

        // buffers
        const int          BLOCK = args["buffer_size"].as<int>();
        std::vector<float> buf(BLOCK);
        sf_count_t         n;

        // inference buffer by buffer
        std::cout << "Starting inferences..." << std::endl;

        std::vector<float> latencies;
        int                frame_counter = 0;

        // deactivate resampling if cvt sample rate == dsp sample rate
        if (model.sample_rate() == gparams.dsp_sample_rate) {
            std::cout << "Deactivating resampling" << std::endl;
            interface.toggle_resampling(false);
        }
        while ((n = input_audio_file.read(buf.data(), BLOCK)) > 0) {
            frame_counter++;
            std::cout << "=== Frame " << frame_counter << " ===" << std::endl;
            std::cout << "processing " << n << " samples" << std::endl;

            auto beg = std::chrono::high_resolution_clock::now();
            interface.run(buf.data(), n);
            auto end = std::chrono::high_resolution_clock::now();

            latencies.push_back(
                std::chrono::duration_cast<std::chrono::milliseconds>(end - beg)
                    .count());  // keep track of per inference latency

            std::copy(
                buf.begin(),
                buf.begin() + n,
                std::back_inserter(output_audio));  // save processed buffer in
                                                    // complete output buffer
        }
        std::cout << "Writing output audio to " << outfilename << "...\n";
        output_audio_file.write(output_audio.data(), output_audio.size());

        npy::npy_data<float> latencies_npy;
        latencies_npy.data  = latencies;
        latencies_npy.shape = {latencies.size()};

        const std::string path{
            std::format("{}/latencies/latencies.npy", aux_out_folder)};
        std::cout << "Writing latencies to " << path << std::endl;
        npy::write_npy(path, latencies_npy);
    }

    std::cout << "Job done" << std::endl;
}