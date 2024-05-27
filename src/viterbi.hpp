#pragma once

#include <cmath>
#include <iostream>
#include <limits>
#include <tuple>
#include <vector>


int solve_viterbi(int const len_time_frame,
                  int const num_kind_phonemes,
                  float const* log_ppg,
                  int const N,
                  std::vector<int> const& phonemes_index,
                  std::vector<std::tuple<float, float, int>>& retval);