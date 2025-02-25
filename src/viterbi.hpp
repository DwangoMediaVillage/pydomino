#pragma once

#include <tuple>
#include <vector>

int viterbi_init(float const* logprobs_transitions, float const* logprobs_blank, std::vector<int> const& token_ids,
                 int const len_timeframes, int const num_transition_vocab, std::vector<float>& log_emission_probs);

std::vector<float> viterbi_forward(int const len_timeframes, int const num_tokens_with_blank,
                                   std::vector<float> const& log_emission_probs, std::vector<bool>& is_transition,
                                   int const min_aligned_time);

int viterbi_backtrace(int const len_timeframes, int const num_tokens_without_blank,
                      std::vector<bool> const& is_transition, int const min_aligned_timeframe,
                      std::vector<int>& transition_timeframes);

int solve_viterbi(int const len_time_frame, int const size_transition_vocab, float const* transition_logprobs,
                  float const* blank_logprobs, int const min_match_timeframes_per_1_phoneme,
                  std::vector<int> const& token_ids, std::vector<int>& transition_timeframes);