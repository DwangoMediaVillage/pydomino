#include <gtest/gtest.h>

#include <Eigen/Core>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "../src/viterbi.hpp"

Eigen::Matrix<float, -1, -1, Eigen::RowMajor> init_by_eigen(
    Eigen::Ref<Eigen::Matrix<float, -1, -1, Eigen::RowMajor>> const logprobs, std::vector<int> const &transition_ids,
    int const blank_id) {
  int const num_timeframes = logprobs.rows();
  int const num_tokens_with_blank = transition_ids.size() * 2 + 1;
  Eigen::Matrix<float, -1, -1, Eigen::RowMajor> retmat =
      Eigen::Matrix<float, -1, -1, Eigen::RowMajor>::Zero(num_timeframes, num_tokens_with_blank);

  for (int i = 0; i < num_timeframes; ++i) {
    for (int j = 0; j < num_tokens_with_blank; ++j) {
      if (j % 2 == 0) {
        retmat(i, j) = logprobs(i, blank_id);
      } else {
        retmat(i, j) = logprobs(i, transition_ids[(j - 1) / 2]);
      }
    }
  }

  return retmat;
}

TEST(Test_ViterbiAlgorithm, test_init) { ; }