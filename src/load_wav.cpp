#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <memory>
#include <cstring>

#include "load_wav.hpp"

template <typename T>
void read_datrum(T& datrum, std::ifstream &ifs)
{
    ifs.read(reinterpret_cast<char*>(&datrum), sizeof(datrum));
}

/// # WAV File Format
/// 'RIFF'  : u32 (4B) RIFF識別子
///   size  : u32 (4B) チャンク サイズ
/// 'WAVE'  : u32 (4B) フォーマット
///
///   'fmt ': u32 (4B) fmt識別子
///       16: u32 (4B) fmtチャンクのバイト数
///        1: u16 (2B) 音声フォーマット
///        1: u16 (2B) チャンネル数
///    16000: u32 (4B) サンプリング周波数
///    32000: u32 (4B) 1 秒あたりバイト数の平均
///        2: u16 (2B) ブロックサイズ
///       16: u16 (2B) ビット／サンプル
///
///   'data': u32 (4B) data識別子
///     size: u32 (4B) dataチャンクのバイト数
///     data: s16[size]
int load_wav(char const* mono_16kHz_16bit_wav_file, std::vector<float>& wav_data) {
    wav_data.clear();

    std::ifstream ifs(mono_16kHz_16bit_wav_file, std::ios::binary);
    if (!ifs) {
        return 1;
    }

    char riff[4];
    read_datrum(riff, ifs);
    if (!std::memcmp(riff, "FFIR", 4)) {
        return -1;
    }

    uint32_t data_size;
    read_datrum(data_size, ifs);

    char wave_identifier[4];
    read_datrum(wave_identifier, ifs);
    if (!std::memcmp(wave_identifier, "EVAW", 4)) {
        return -2;
    }

    char fmt_identifier[4];
    read_datrum(fmt_identifier, ifs);
    if (!std::memcmp(fmt_identifier, " tmf", 4)) {
        return -3;
    }

    uint32_t fmt_chunk_num_bit;
    read_datrum(fmt_chunk_num_bit, ifs);
    if (fmt_chunk_num_bit != 16) {
        return -4;
    }

    // 音声フォーマット (2Byte)
    uint16_t audio_format;
    read_datrum(audio_format, ifs);
    if (audio_format != 1) {
        return -5;
    }

    // チャンネル数 (2Byte)
    uint16_t num_channels;
    read_datrum(num_channels, ifs);
    if (num_channels != 1) {
        return -6;
    }

    // Sampling rate (4Byte)
    // C++14から comma が掛けます
    uint32_t sample_rate;
    read_datrum(sample_rate, ifs);
    if (sample_rate != 16'000) {
        return -7;
    }

    // 1秒あたりのByte数の平均 (4Byte) 2 * sample_rate
    uint32_t num_Byte_per_second;
    read_datrum(num_Byte_per_second, ifs);
    if (num_Byte_per_second != 32'000) {
        return -8;
    }

    // ブロックサイズ 2Byte
    uint16_t block_size;
    read_datrum(block_size, ifs);
    if (block_size != 2) { // Byte表記なので2。16ではない
        return -9;
    }

    // 1サンプルに必要なビット数 2Byte
    uint16_t num_bits_per_sample;
    read_datrum(num_bits_per_sample, ifs);
    if (num_bits_per_sample != 16) {
        return -10;
    }

    while (ifs)
    {
        // 'data': u32 (4B) data識別子
        char chk_identifier[4];
        read_datrum(chk_identifier, ifs);
        if (ifs.eof()) {
            break;
        }

        // size: u32 (4B) dataチャンクのバイト数
        uint32_t size;
        read_datrum(size, ifs);

        if (!std::memcmp(chk_identifier, "atad", 4)) {
            // std::cout << chk_identifier << ": " << size << std::endl;
            ifs.seekg(size, std::ios_base::cur);
            continue;
        }

        size /= 2;

        std::unique_ptr<short[]> data(new short[size]);
        ifs.read(reinterpret_cast<char*>(data.get()), size * sizeof(short));
        wav_data.resize(size);
#if 0
        for (int s = 0; s < size; ++s) {
            wav_data[s] = data[s] / float(32768);
        }
#else
        std::transform(
            data.get(), data.get() + size,
            wav_data.begin(),
            [](short x) -> float { return x / float(32768); });
#endif
    }

    return 0;
}
