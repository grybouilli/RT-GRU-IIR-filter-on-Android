#include <oboe/Oboe.h>

int32_t acquire_audio_stream(
    oboe::AudioStreamBuilder& builder,
    oboe::Direction direction, 
    oboe::PerformanceMode performance_mode, 
    oboe::SharingMode sharing_mode, 
    oboe::AudioFormat audio_format, 
    oboe::ChannelCount channel_count
)
{
    // We'll open with a default sample rate first, then read back what was chosen
    builder.setDirection( direction )
           ->setPerformanceMode( performance_mode )
           ->setSharingMode( sharing_mode )
           ->setFormat( audio_format )
           ->setChannelCount( channel_count );

    // Open stream once to discover the actual sample rate the device picked
    oboe::ManagedStream probe_stream;
    oboe::Result result = builder.openManagedStream(probe_stream);
    if (result != oboe::Result::OK) {
        fprintf(stderr, "Failed to open probe stream: %s\n", oboe::convertToText(result));
        return -1;
    }
    int32_t sample_rate = probe_stream->getSampleRate();
    probe_stream->close();

    printf("Device sample rate: %d Hz\n", sample_rate);
    return sample_rate;
}