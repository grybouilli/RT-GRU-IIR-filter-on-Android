#pragma once

#include <oboe/Oboe.h>

int32_t acquire_audio_stream(
    oboe::AudioStreamBuilder& builder,
    oboe::Direction direction, 
    oboe::PerformanceMode performance_mode, 
    oboe::SharingMode sharing_mode, 
    oboe::AudioFormat audio_format, 
    oboe::ChannelCount channel_count
);