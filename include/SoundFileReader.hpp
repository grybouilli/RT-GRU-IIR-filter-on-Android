#pragma once

#include <sndfile_utils.hpp>
#include <string>

template <typename T, int Format, int IntType = 0>
class SoundFileReader {
   public:
    SoundFileReader(const std::string filepath,
                    const int         sample_rate,
                    const int         channels) :
        m_info{.samplerate = sample_rate,
               .channels   = channels,
               .format     = type_to_sf_type<T, IntType>() | Format},
        m_file{sf_open(filepath.c_str(), SFM_READ, &m_info)} {}
    ~SoundFileReader() { sf_close(m_file); }

    sf_count_t read(T* data, const size_t length) {
        if constexpr (std::same_as<T, float>) {
            return sf_read_float(m_file, data, length);
        } else if constexpr (std::same_as<T, double>) {
            return sf_read_double(m_file, data, length);
        } else if constexpr (std::same_as<T, int>) {
            return sf_read_int(m_file, data, length);
        }
    }

    int frames() const& { return m_info.frames; }
    int channels() const& { return m_info.channels; }

   private:
    SF_INFO  m_info;
    SNDFILE* m_file;
};