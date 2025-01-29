#include "viterbi.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <tuple>
#include <vector>

/**
 * @brief log_emission_probs を生成する関数
 *
 * @param len_timeframes
 * @param num_tokens_with_blank
 * @param logprobs_output_by_onnxnetwork
 * @param phonemes_index
 * @param num_transition_vocab
 * @param blank_id
 * @param log_emission_probs
 * @return int
 */
int viterbi_init(int const len_timeframes, int const num_tokens_with_blank, float const* logprobs_output_by_onnxnetwork,
                 std::vector<int> const& phonemes_index, int const num_transition_vocab, int const blank_id,
                 std::vector<float>& log_emission_probs) {
  for (std::size_t t = 0; t < len_timeframes; ++t) {
    for (std::size_t i = 0; i < num_tokens_with_blank; ++i) {
      if (i % 2 == 0) {
        log_emission_probs[t * num_tokens_with_blank + i] =
            logprobs_output_by_onnxnetwork[t * num_transition_vocab + blank_id];
      } else {
        log_emission_probs[t * num_tokens_with_blank + i] =
            logprobs_output_by_onnxnetwork[t * num_transition_vocab + phonemes_index[(i - 1) / 2]];
      }
    }
  }
}

/**
 * @brief
 *
 * @param len_timeframes 時間フレーム数
 * @param num_tokens_with_blank blankトークンを含んだうえでの入力トークン数
 * @param log_emission_probs log_emission_probs[t * num_tokens_with_blank + i]: 時刻 t
 * 時刻tでi番目の音素遷移が起きる確率
 * @param is_transition is_transition[t * num_tokens_with_blank + i] = true: 時刻 t
 * 時刻tでi番目の音素遷移が起きた
 * @param min_aligned_time 1音素に割り当てる最小時間フレーム数
 * @return int
 */
int viterbi_forward(int const len_timeframes, int const num_tokens_with_blank,
                    std::vector<float> const log_emission_probs, std::vector<bool> is_transition,
                    int const min_aligned_time) {}

/**
 * @brief backtrace部分
 *
 * @param len_timeframes 時間フレーム数
 * @param num_tokens_with_blank blankトークンを含んだうえでの入力トークン数
 * @param is_transition is_transition[t * num_tokens_with_blank + i] = true: 時刻 t
 * 時刻tでi番目の音素遷移が起きた
 * @param min_aligned_time 1音素に割り当てる最小時間フレーム数
 * @param transition_timeframes 結果を入力する変数
 * @return int
 */
int viterbi_backtrace(int const len_timeframes, int const num_tokens_with_blank, std::vector<bool> const is_transition,
                      int const min_aligned_time, std::vector<int>& transition_timeframes) {
  int t = len_timeframes - 1;
  int i = num_tokens_with_blank - 2;
  while (t >= 0) {
    if (is_transition[t * num_tokens_with_blank + i]) {
      transition_timeframes[i] = t;
      t -= min_aligned_time;
      i -= 2;
    } else {
      --t;
    }
  }
  if (i != 0) {
    std::cerr << "[WARN] could not alignement" << std::endl;
    return 1;
  }

  return 0;
}

int solve_viterbi(int const len_time_frame,
                  int const num_transition_vocab,  // num_vocab
                  int const blank_id,
                  float const* logprobs_output_by_onnxnetwork,   // len_time_frame x num_kind_phonemes
                  int const min_match_timeframes_per_1_phoneme,  // min match frame length per 1 phoneme
                  std::vector<int> const& phonemes_index,        // a int sequence
                  std::vector<int>& transition_timeframes) {
  int const num_tokens_with_blank = phonemes_index.size() * 2 + 1;
  std::vector<float> log_emission_probs(len_time_frame * num_tokens_with_blank);
  viterbi_init(len_time_frame, num_tokens_with_blank, logprobs_output_by_onnxnetwork, phonemes_index,
               num_transition_vocab, blank_id, log_emission_probs);
  std::vector<bool> is_transition(log_emission_probs.size(), false);
  viterbi_forward(len_time_frame, num_tokens_with_blank, log_emission_probs, is_transition,
                  min_match_timeframes_per_1_phoneme);
  viterbi_backtrace(len_time_frame, num_tokens_with_blank, is_transition, min_match_timeframes_per_1_phoneme,
                    transition_timeframes);
  return 0;
}
