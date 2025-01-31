#include "viterbi.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <tuple>
#include <vector>

/**
 * @brief
 *
 * @param logprobs_transitions
 * @param logprobs_blank
 * @param phonemes_index
 * @param len_timeframes
 * @param num_transition_vocab
 * @param log_emission_probs
 * @return int
 */
int viterbi_init(float const* logprobs_transitions, float const* logprobs_blank, std::vector<int> const& token_ids,
                 int const len_timeframes, int const num_transition_vocab, std::vector<float>& log_emission_probs) {
  int const num_tokens_with_blank = token_ids.size() * 2 + 1;
  for (std::size_t t = 0; t < len_timeframes; ++t) {
    for (std::size_t i = 0; i < num_tokens_with_blank; ++i) {
      if (i % 2 == 0) {
        log_emission_probs[t * num_tokens_with_blank + i] = logprobs_blank[t];
      } else {
        log_emission_probs[t * num_tokens_with_blank + i] =
            logprobs_transitions[t * num_transition_vocab + token_ids[(i - 1) / 2]];
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
std::vector<float> viterbi_forward(int const len_timeframes, int const num_tokens_with_blank,
                                   std::vector<float> const& log_emission_probs, std::vector<bool>& is_transition,
                                   int const min_aligned_time) {
  int const num_tokens_without_blank = (num_tokens_with_blank - 1) / 2;  // blankを含まない音素遷移トークン数
  std::vector<float> forward_logprobs(log_emission_probs.size(), -std::numeric_limits<float>::infinity());
  std::fill(is_transition.begin(), is_transition.end(), false);

  forward_logprobs[0] = log_emission_probs[0];
  forward_logprobs[1] = log_emission_probs[1];
  for (int t = 1; t < len_timeframes; ++t) {
    forward_logprobs[t * num_tokens_with_blank] =
        forward_logprobs[(t - 1) * num_tokens_with_blank] + log_emission_probs[t * num_tokens_with_blank];
  }

  for (int t = 1; t < len_timeframes; ++t) {
    forward_logprobs[t * num_tokens_with_blank + 1] =
        forward_logprobs[(t - 1) * num_tokens_with_blank] + log_emission_probs[t * num_tokens_with_blank + 1];
  }

  for (int token_idx = 3; token_idx < num_tokens_with_blank; token_idx += 2) {
    for (int t = min_aligned_time * (token_idx - 1) / 2; t < len_timeframes; ++t) {
      // i - 1番目の音素遷移が t - N フレーム目で遷移が起こっていたときの前向き確率
      float forward_logprob_if_transit_Nframes_ago =
          forward_logprobs[(t - min_aligned_time) * num_tokens_with_blank + token_idx - 2];
      for (int s = t - min_aligned_time + 1; s < t; ++s) {
        forward_logprob_if_transit_Nframes_ago += log_emission_probs[s * num_tokens_with_blank + token_idx - 1];
      }

      // i - 1番目の音素遷移が t - N フレーム目より前で遷移が起こっていたときの前向き確率
      float forward_logprob_if_transit_before_Nframes;
      if (t >= 2) {
        forward_logprob_if_transit_before_Nframes = forward_logprobs[(t - 2) * num_tokens_with_blank + token_idx - 1] +
                                                    log_emission_probs[(t - 1) * num_tokens_with_blank + token_idx - 1];
      } else {
        forward_logprob_if_transit_before_Nframes = -std::numeric_limits<float>::infinity();
      }

      is_transition[(t - min_aligned_time) * num_tokens_without_blank + (token_idx - 3) / 2] =
          (forward_logprob_if_transit_Nframes_ago > forward_logprob_if_transit_before_Nframes);
      forward_logprobs[t * num_tokens_with_blank + token_idx] =
          std::max(forward_logprob_if_transit_Nframes_ago, forward_logprob_if_transit_before_Nframes) +
          log_emission_probs[t * num_tokens_with_blank + token_idx];
      forward_logprobs[(t - 1) * num_tokens_with_blank + token_idx - 1] =
          forward_logprobs[t * num_tokens_with_blank + token_idx] -
          log_emission_probs[t * num_tokens_with_blank + token_idx];
    }
  }

  // 最後のblank
  int last_blank_idx = num_tokens_with_blank - 1;
  for (int t = min_aligned_time; t < len_timeframes; ++t) {
    forward_logprobs[t * num_tokens_with_blank + last_blank_idx] =
        std::max(forward_logprobs[(t - 1) * num_tokens_with_blank + last_blank_idx - 1],
                 forward_logprobs[(t - 1) * num_tokens_with_blank + last_blank_idx]) +
        log_emission_probs[t * num_tokens_with_blank + last_blank_idx];
    is_transition[(t - 1) * num_tokens_without_blank + num_tokens_without_blank - 1] =
        forward_logprobs[(t - 1) * num_tokens_with_blank + last_blank_idx - 1] >
        forward_logprobs[(t - 1) * num_tokens_with_blank + last_blank_idx];
  }
  is_transition[(len_timeframes - 1) * num_tokens_without_blank + num_tokens_without_blank - 1] =
      forward_logprobs[(len_timeframes - 1) * num_tokens_with_blank + last_blank_idx - 1] >
      forward_logprobs[(len_timeframes - 1) * num_tokens_with_blank + last_blank_idx];

  return forward_logprobs;
}

/**
 * @brief backtrace部分
 *
 * @param len_timeframes 時間フレーム数
 * @param num_tokens_without_blank 入力音素遷移トークン数
 * @param is_transition is_transition[t * num_tokens_without_blank + i] = true: 時刻 t
 * 時刻tでi番目の音素遷移が起きた
 * @param min_aligned_time 1音素に割り当てる最小時間フレーム数
 * @param transition_timeframes 結果を入力する変数
 * @return int
 */
int viterbi_backtrace(int const len_timeframes, int const num_tokens_without_blank,
                      std::vector<bool> const& is_transition, int const min_aligned_time,
                      std::vector<int>& transition_timeframes) {
  int t = len_timeframes - 1;
  int i = num_tokens_without_blank - 1;
  while (t >= 0 && i >= 0) {
    if (is_transition[t * num_tokens_without_blank + i]) {
      transition_timeframes[i] = t;
      t -= min_aligned_time;
      --i;
    } else {
      --t;
    }
  }

  return 0;
}

int solve_viterbi(int const len_time_frame,
                  int const size_transition_vocab,               // size_transition_vocab
                  float const* transition_logprobs,              // len_time_frame x size_transition_vocab
                  float const* blank_logprobs,                   // len_time_frame
                  int const min_match_timeframes_per_1_phoneme,  // min match frame length per 1 phoneme
                  std::vector<int> const& token_ids,             // a int sequence
                  std::vector<int>& transition_timeframes) {
  int const num_tokens_with_blank = token_ids.size() * 2 + 1;
  std::vector<float> log_emission_probs(len_time_frame * num_tokens_with_blank);
  viterbi_init(transition_logprobs, blank_logprobs, token_ids, len_time_frame, size_transition_vocab,
               log_emission_probs);
  std::vector<bool> is_transition(log_emission_probs.size(), false);
  viterbi_forward(len_time_frame, num_tokens_with_blank, log_emission_probs, is_transition,
                  min_match_timeframes_per_1_phoneme);
  viterbi_backtrace(len_time_frame, token_ids.size(), is_transition, min_match_timeframes_per_1_phoneme,
                    transition_timeframes);
  return 0;
}
