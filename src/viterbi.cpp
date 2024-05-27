#include <cmath>
#include <iostream>
#include <limits>
#include <tuple>
#include <vector>
#include "viterbi.hpp"


int solve_viterbi(int const len_time_frame,
                  int const num_kind_phonemes, // num_vocab
                  float const* log_ppg, // len_time_frame x num_kind_phonemes
                  int const N, // min match frame length per 1 phoneme
                  std::vector<int> const& phonemes_index, // a int sequence
                  std::vector<std::tuple<float, float, int>>& retval) {
    if (len_time_frame < 2) {
        std::runtime_error("");
    }

    std::size_t const phoneme_length = phonemes_index.size();
    if (len_time_frame < phoneme_length) {
        std::runtime_error("len_time_frame must be longer than phoneme_length.");
    }
    // dist[t * phoneme_length + i] = x : 時刻tでi番目の音素が読まれているときのその対数尤度が x
    std::vector<float> dist(len_time_frame * phoneme_length);
    for (std::size_t t = 0; t < len_time_frame; ++t) {
        for (std::size_t i = 0; i < phoneme_length; ++i) {
            dist[t * phoneme_length + i] = log_ppg[t * num_kind_phonemes + phonemes_index[i]];
        }
    }

    int const N_ = std::min(static_cast<int>(len_time_frame / phoneme_length), N);

    // 初期化
    std::vector<float> forward_logprobs(dist.size(), -std::numeric_limits<float>::infinity());
    forward_logprobs[0] = dist[0];
    for (std::size_t t = 1; t < len_time_frame; ++t) {
        forward_logprobs[t * phoneme_length + 0] = forward_logprobs[(t - 1) * phoneme_length + 0] + dist[t * phoneme_length + 0];
    }

    // is_Nth_frame[t * phoneme_length + i] = True: 時刻tでi番目の音素がNフレーム目であるということ
    std::vector<bool> is_Nth_frame(dist.size(), false);

    // DP部分
    for (std::size_t t = N_; t < len_time_frame; ++t) {
        for (std::size_t i = 1; i < phoneme_length; ++i) {
            // 音素継続
            float const logprobs_if_this_is_over_Nth_frame = forward_logprobs[(t - 1) * phoneme_length + i] + dist[t * phoneme_length + i];
            // N フレーム前に音素遷移
            float logprobs_if_this_is_Nth_frame = forward_logprobs[(t - N_) * phoneme_length + (i - 1)];
            for (int n = 0; n < N_; ++n) {
                logprobs_if_this_is_Nth_frame += dist[(t - n) * phoneme_length + i];
            }

            if (logprobs_if_this_is_Nth_frame > logprobs_if_this_is_over_Nth_frame) {
                forward_logprobs[t * phoneme_length + i] = logprobs_if_this_is_Nth_frame;
                is_Nth_frame[t * phoneme_length + i] = true;
            } else {
                forward_logprobs[t * phoneme_length + i] = logprobs_if_this_is_over_Nth_frame;
                is_Nth_frame[t * phoneme_length + i] = false;
            }
        }
    }

    // Backtrace部分
    std::vector<int> alignment(len_time_frame, -1);
    int t = len_time_frame - 1;
    int p = phoneme_length - 1;
    while (t >= 0) {
        if (is_Nth_frame[t * phoneme_length + p]) {
            for (int n = 0; n < N_; ++n) {
                alignment[t--] = p;
            }
            --p;
        } else {
            alignment[t--] = p;
        }
    }
    if (p != 0) {
        std::cerr << "[WARN] could not alignement" << std::endl;
    }

    float begin_sec = 0.0f;
    float fps = 100.0f;
    int i = 0;
    retval.clear();
    for (auto phoneme = alignment.begin(); phoneme != alignment.end(); ++phoneme, ++i) {
        if (i == alignment.size() - 1) {
            retval.push_back(std::make_tuple(begin_sec, (i + 1) / fps, *phoneme));
        } else if (*phoneme != alignment[i + 1]) {
            retval.push_back(std::make_tuple(begin_sec, (i + 1) / fps, *phoneme));
            begin_sec = (i + 1) / fps;
        }
    }
    
    return 0;
}
