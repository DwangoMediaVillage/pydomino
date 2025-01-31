
#include <H5Cpp.h>
#include <gtest/gtest.h>

#include <Eigen/Core>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "../src/viterbi.hpp"

float* loadHDF5(const std::string& filename, int& rows, int& cols) {
  try {
    // HDF5 ファイルを開く
    H5::H5File file(filename, H5F_ACC_RDONLY);

    // データセットを開く
    H5::DataSet dataset = file.openDataSet("array");

    // データの次元を取得
    H5::DataSpace dataspace = dataset.getSpace();
    hsize_t dims[2];  // 2D 行列
    dataspace.getSimpleExtentDims(dims, nullptr);
    rows = dims[0], cols = dims[1];

    // データを格納するメモリを確保
    float* data = new float[rows * cols];

    // データを読み込む
    dataset.read(data, H5::PredType::NATIVE_FLOAT);
    return data;
  } catch (H5::Exception& e) {
    std::cerr << "HDF5 error: " << e.getDetailMsg() << std::endl;
    return nullptr;
  }
}

float* loadHDF5(const std::string& filename, int& rows) {
  try {
    // HDF5 ファイルを開く
    H5::H5File file(filename, H5F_ACC_RDONLY);

    // データセットを開く
    H5::DataSet dataset = file.openDataSet("array");

    // データの次元を取得
    H5::DataSpace dataspace = dataset.getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims, nullptr);
    rows = dims[0];

    // データを格納するメモリを確保
    float* data = new float[rows];

    // データを読み込む
    dataset.read(data, H5::PredType::NATIVE_FLOAT);
    return data;
  } catch (H5::Exception& e) {
    std::cerr << "HDF5 error: " << e.getDetailMsg() << std::endl;
    return nullptr;
  }
}

Eigen::Matrix<float, -1, -1, Eigen::RowMajor> loadHDF5(std::string const& transition_logprobs_HDF5path,
                                                       std::string const& blank_logprobs_HDF5path) {
  try {
    H5::H5File file(transition_logprobs_HDF5path, H5F_ACC_RDONLY);
    H5::DataSet dataset = file.openDataSet("array");

    // データの次元を取得
    H5::DataSpace dataspace = dataset.getSpace();
    hsize_t dims[2];  // 2D 行列
    dataspace.getSimpleExtentDims(dims, nullptr);
    size_t rows = dims[0], cols = dims[1];

    Eigen::Matrix<float, -1, -1, Eigen::RowMajor> transition_logprobs(rows, cols);
    dataset.read(transition_logprobs.data(), H5::PredType::NATIVE_FLOAT);
    return transition_logprobs;
  } catch (H5::Exception& e) {
    std::cerr << "HDF5 error: " << e.getDetailMsg() << std::endl;
    return Eigen::Matrix<float, -1, -1, Eigen::RowMajor>();  // エラー時は空行列
  }
}

Eigen::Matrix<float, -1, -1, Eigen::RowMajor> init_by_eigen(
    Eigen::Ref<Eigen::Matrix<float, -1, -1, Eigen::RowMajor>> const logprobs, std::vector<int> const& transition_ids,
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

void test_init_func(std::string transition_logprobs_HDF5path, std::string blank_logprobs_HDF5path,
                    std::vector<int> const token_ids) {
  int len_timeframes, num_transition_vocabs, blank_len_timeframes;
  float const* transition_logprobs = loadHDF5(transition_logprobs_HDF5path, len_timeframes, num_transition_vocabs);
  float const* blank_logprobs = loadHDF5(blank_logprobs_HDF5path, blank_len_timeframes);

  Eigen::Matrix<float, -1, -1, Eigen::RowMajor> transition_logprobs_matrix(len_timeframes, num_transition_vocabs);
  Eigen::Matrix<float, -1, -1, Eigen::RowMajor> blank_logprobs_matrix(len_timeframes, 1);
  Eigen::Matrix<float, -1, -1, Eigen::RowMajor> input_matrix(len_timeframes, num_transition_vocabs + 2);
  input_matrix << blank_logprobs_matrix << transition_logprobs << blank_logprobs_matrix;
  Eigen::Matrix<float, -1, -1, Eigen::RowMajor> expected = init_by_eigen(input_matrix, token_ids, 557);
  ASSERT_EQ(len_timeframes, blank_len_timeframes);
  std::vector<float> log_emission_probs(len_timeframes * (token_ids.size() * 2 + 1));
  viterbi_init(transition_logprobs, blank_logprobs, token_ids, len_timeframes, 556, log_emission_probs);

  for (int i = 0; i < len_timeframes; ++i) {
    for (int j = 0; j < token_ids.size() * 2 + 1; ++j) {
      ASSERT_EQ(log_emission_probs[i * (token_ids.size() * 2 + 1) + j], expected(i, j));
    }
  }
}

TEST(Test_ViterbiAlgorithm, test_init) {
  std::filesystem::current_path("../../");
  test_init_func("tests/prediction_logprobs/dowaNgo_transition_logprobs.h5",
                 "tests/prediction_logprobs/dowaNgo_blank_logprobs.h5", {531, 81, 379, 196, 237, 461, 95, 387});
  test_init_func("tests/prediction_logprobs/ishIkI_transition_logprobs.h5",
                 "tests/prediction_logprobs/ishIkI_blank_logprobs.h5", {550, 258, 131, 403, 110, 417});
  test_init_func("tests/prediction_logprobs/tasuuketsU_transition_logprobs.h5",
                 "tests/prediction_logprobs/tasuuketsU_blank_logprobs.h5",
                 {546, 182, 222, 135, 292, 108, 331, 125, 447});
}

#if 0

std::tuple<Eigen::Matrix<bool, -1, -1, Eigen::RowMajor>, Eigen::Matrix<float, -1, -1, Eigen::RowMajor>>
forward_by_eigen(Eigen::Ref<Eigen::Matrix<float, -1, -1, Eigen::RowMajor>> const dist, int const minimum_align_length) {
  int const num_timeframes = dist.rows();
  int const num_tokens_with_blank = dist.cols();

  Eigen::Matrix<bool, -1, -1, Eigen::RowMajor> is_transition =
      Eigen::Matrix<bool, -1, -1, Eigen::RowMajor>::Zero(num_timeframes, (num_tokens_with_blank - 1) / 2);
  Eigen::Matrix<float, -1, -1, Eigen::RowMajor> forward_logprobs =
      Eigen::Matrix<float, -1, -1, Eigen::RowMajor>::Constant(num_timeframes, num_tokens_with_blank,
                                                              -std::numeric_limits<float>::infinity());
  // forward_logprobs.setConstant(-std::numeric_limits<float>::infinity());

  // 最初のblank
  forward_logprobs(0, 0) = dist(0, 0);
  for (int t = 1; t < num_timeframes; ++t) {
    forward_logprobs(t, 0) = forward_logprobs(t - 1, 0) + dist(t, 0);
  }

  // 最初の遷移トークン
  forward_logprobs(0, 1) = dist(0, 1);
  for (int t = 1; t < num_timeframes; ++t) {
    forward_logprobs(t, 1) = forward_logprobs(t - 1, 0) + dist(t, 1);
  }

  // 2個目のblank ~ 最後の遷移トークン
  for (int i = 3; i < num_tokens_with_blank - 1; i = i + 2) {
    for (int t = minimum_align_length; t < num_timeframes; ++t) {
      // i - 1番目の音素遷移が t - N フレーム目で遷移が起こっていたときの前向き確率
      float forward_logprob_if_transit_Nframes_ago = forward_logprobs(t - minimum_align_length, i - 2);
      for (int s = t - minimum_align_length + 1; s < t; ++s) {
        forward_logprob_if_transit_Nframes_ago += dist(s, i - 1);
      }

      // i - 1番目の音素遷移が t - N フレーム目より前で遷移が起こっていたときの前向き確率
      float forward_logprob_if_transit_before_Nframes =
          (t - 2 >= 0) ? forward_logprobs(t - 2, i - 1) + dist(t - 1, i - 1) : -std::numeric_limits<float>::infinity();

      is_transition(t - minimum_align_length, (i - 1) / 2 - 1) =
          forward_logprob_if_transit_Nframes_ago > forward_logprob_if_transit_before_Nframes;
      forward_logprobs(t, i) =
          std::max(forward_logprob_if_transit_Nframes_ago, forward_logprob_if_transit_before_Nframes) + dist(t, i);
      forward_logprobs(t - 1, i - 1) = forward_logprobs(t, i) - dist(t, i);
    }
  }

  // 最後のblank
  int last_blank_idx = num_tokens_with_blank - 1;
  for (int t = minimum_align_length; t < num_timeframes; ++t) {
    forward_logprobs(t, last_blank_idx) =
        std::max(forward_logprobs(t - 1, last_blank_idx - 1), forward_logprobs(t - 1, last_blank_idx)) +
        dist(t, last_blank_idx);
    is_transition(t - 1, last_blank_idx / 2 - 1) =
        forward_logprobs(t - 1, last_blank_idx - 1) > forward_logprobs(t - 1, last_blank_idx);
  }
  is_transition(num_timeframes - 1, last_blank_idx / 2 - 1) =
      forward_logprobs(num_timeframes - 1, last_blank_idx - 1) > forward_logprobs(num_timeframes - 1, last_blank_idx);

  return std::make_tuple(is_transition, forward_logprobs);
}

void test_forward_func(std::string HDF5path, std::vector<int> const token_ids) {
  int rows, cols;
  float const* y = loadHDF5(HDF5path, rows, cols);
  Eigen::Matrix<float, -1, -1, Eigen::RowMajor> m = loadHDF5(HDF5path);

  int const blank_id = 557;
  Eigen::Matrix<float, -1, -1, Eigen::RowMajor> log_emission_probs_by_eigen = init_by_eigen(m, token_ids, blank_id);
  std::vector<float> log_emission_probs(rows * (cols * 2 + 1));
  int const num_tokens_with_blank = token_ids.size() * 2 + 1;
  viterbi_init(rows, y, token_ids, 558, blank_id, log_emission_probs);

  for (int minimum_align_length = 1; minimum_align_length < 8; ++minimum_align_length) {
    std::tuple<Eigen::Matrix<bool, -1, -1, Eigen::RowMajor>, Eigen::Matrix<float, -1, -1, Eigen::RowMajor>>
        result_eigen = forward_by_eigen(log_emission_probs_by_eigen, minimum_align_length);

    std::vector<bool> is_transition(rows * token_ids.size());
    std::vector<float> const forward_logprobs =
        viterbi_forward(rows, token_ids.size() * 2 + 1, log_emission_probs, is_transition, minimum_align_length);

    Eigen::Matrix<bool, -1, -1, Eigen::RowMajor> expected_is_transit = std::get<0>(result_eigen);
    Eigen::Matrix<float, -1, -1, Eigen::RowMajor> expected_forward_logprobs = std::get<1>(result_eigen);

    for (int j = 0; j < token_ids.size(); ++j) {
      for (int t = 0; t < rows; ++t) {
        ASSERT_EQ(is_transition[t * token_ids.size() + j], expected_is_transit(t, j))
            << "Error: Emerged at t = " << t << ", token_idx = " << j << ", N=" << minimum_align_length;
      }
    }
    for (int j = 0; j < num_tokens_with_blank; ++j) {
      for (int t = 0; t < rows; ++t) {
        ASSERT_EQ(forward_logprobs[t * num_tokens_with_blank + j], expected_forward_logprobs(t, j))
            << "Error: Emerged at t = " << t << ", token_idx = " << j << ", N=" << minimum_align_length;
      }
    }
  }
}

TEST(Test_ViterbiAlgorithm, test_forward) {
  test_forward_func("tests/prediction_logprobs/dowaNgo_transition_logprobs.h5",
                    "tests/prediction_logprobs/dowaNgo_blank_logprobs.h5", {531, 81, 379, 196, 237, 461, 95, 387});
  test_forward_func("tests/prediction_logprobs/ishIkI_transition_logprobs.h5",
                    "tests/prediction_logprobs/ishIkI_blank_logprobs.h5", {550, 258, 131, 403, 110, 417});
  test_forward_func("tests/prediction_logprobs/tasuuketsU_transition_logprobs.h5",
                    "tests/prediction_logprobs/tasuuketsU_blank_logprobs.h5",
                    {546, 182, 222, 135, 292, 108, 331, 125, 447});
}

/**
 * @brief viterbiアルゴリズムのbacktrace部分を定義する
 *
 * @param is_transit shape = (num_timeframes, num_tokens_without_blank). is_transit[t, i] = true
 * はそこでi番目の音素遷移が起こったことを表す
 * @param minimum_align_length 音素遷移Viterbiにおいて、最低割り当てフレーム数。
 * @return std::vector<int> length = num_tokens_without_blank.
 */
std::vector<int> backtrace_by_eigen(Eigen::Ref<Eigen::Matrix<bool, -1, -1, Eigen::RowMajor>> const is_transit,
                                    int minimum_align_length) {
  int const num_tokens_without_blank = is_transit.cols();
  std::vector<int> transition_timeframes(num_tokens_without_blank, 0);
  int t = is_transit.rows() - 1;
  int i = num_tokens_without_blank - 1;
  while (t > -1) {
    if (is_transit(t, i)) {
      transition_timeframes[i--] = t;
      t -= minimum_align_length;
    } else {
      --t;
    }
    if (i < 0) {
      break;
    }
  }
  return transition_timeframes;
}

void test_backtrace_func(std::string HDF5path, std::vector<int> const token_ids) {
  int rows, cols;
  float const* y = loadHDF5(HDF5path, rows, cols);
  const int len_timeframe = rows;
  Eigen::Matrix<float, -1, -1, Eigen::RowMajor> m = loadHDF5(HDF5path);

  int const blank_id = 557;
  Eigen::Matrix<float, -1, -1, Eigen::RowMajor> log_emission_probs_by_eigen = init_by_eigen(m, token_ids, blank_id);
  std::vector<float> log_emission_probs(len_timeframe * (cols * 2 + 1));
  int const num_tokens_with_blank = token_ids.size() * 2 + 1;
  viterbi_init(len_timeframe, y, token_ids, 558, blank_id, log_emission_probs);

  for (int minimum_align_length = 1; minimum_align_length < 8; ++minimum_align_length) {
    std::tuple<Eigen::Matrix<bool, -1, -1, Eigen::RowMajor>, Eigen::Matrix<float, -1, -1, Eigen::RowMajor>>
        result_eigen = forward_by_eigen(log_emission_probs_by_eigen, minimum_align_length);
    std::vector<bool> is_transition(len_timeframe * token_ids.size());
    viterbi_forward(len_timeframe, token_ids.size() * 2 + 1, log_emission_probs, is_transition, minimum_align_length);
    Eigen::Matrix<bool, -1, -1, Eigen::RowMajor> is_transit_by_eigen = std::get<0>(result_eigen);

    std::vector<int> expected_timeframes = backtrace_by_eigen(is_transit_by_eigen, minimum_align_length);
    std::vector<int> output_timeframes(token_ids.size(), 0);
    viterbi_backtrace(len_timeframe, token_ids.size(), is_transition, minimum_align_length, output_timeframes);

    ASSERT_EQ(output_timeframes.size(), expected_timeframes.size());
    for (int i = 0; i < expected_timeframes.size(); ++i) {
      ASSERT_EQ(output_timeframes[i], expected_timeframes[i]);
    }
  }
}

TEST(Test_ViterbiAlgorithm, test_backtrace) {
  test_backtrace_func("tests/prediction_logprobs/dowaNgo_transition_logprobs.h5",
                      "tests/prediction_logprobs/dowaNgo_blank_logprobs.h5", {531, 81, 379, 196, 237, 461, 95, 387});
  test_backtrace_func("tests/prediction_logprobs/ishIkI_transition_logprobs.h5",
                      "tests/prediction_logprobs/ishIkI_blank_logprobs.h5", {550, 258, 131, 403, 110, 417});
  test_backtrace_func("tests/prediction_logprobs/tasuuketsU_transition_logprobs.h5",
                      "tests/prediction_logprobs/tasuuketsU_blank_logprobs.h5",
                      {546, 182, 222, 135, 292, 108, 331, 125, 447});
}
#endif