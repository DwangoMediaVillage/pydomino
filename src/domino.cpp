/* Pythonライブラリとコマンドコンソール両方のインタフェースを書く場所 */
#include "domino.hpp"

#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "phoneme_transition.hpp"
#include "viterbi.hpp"

namespace domino {
Aligner::Aligner(std::string const &path, int const N)
    : env_(),
      session_options_(),
      session_(env_, std::filesystem::path(path).c_str(), session_options_),
      memory_info_(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)),
      run_options_(),
      N_(N) {
  std::cout << "path: " << path << std::endl;
}

Aligner::~Aligner() { this->release(); }

void Aligner::release() {
  run_options_.release();
  memory_info_.release();
  session_.release();
  env_.release();
}

std::vector<std::tuple<double, double, std::string>> Aligner::align_phonemes(Eigen::Ref<Eigen::VectorXf> const wav_data,
                                                                             std::string const &phonemes, int N) {
  return this->align(wav_data.data(), wav_data.size(), Aligner::read_phonemes(phonemes), N);
}

std::vector<std::tuple<double, double, std::string>> Aligner::align(float const *wav_data,
                                                                    std::size_t const wav_data_size,
                                                                    std::vector<int> const &token_ids, int N) {
  if (N < 1) {
    N = N_;
  }

  constexpr char const *const input_names[] = {"x"};
  constexpr char const *const output_names[] = {"y"};
  // NOTE: C++17以上が必須
  std::array<std::int64_t, 2> const wav_data_shape = {1, static_cast<std::int64_t>(wav_data_size)};
  Ort::Value inputs[] = {
      Ort::Value::CreateTensor(memory_info_, const_cast<float *>(wav_data), wav_data_size, wav_data_shape.data(),
                               wav_data_shape.size()),
  };
  std::vector<Ort::Value> const outputs =
      session_.Run(run_options_, input_names, inputs, std::size(input_names), output_names, std::size(output_names));
  // output y.shape = (1, len_time_frame, num_kind_phonemes)
  float const *const y = outputs[0].GetTensorData<float>();
  auto const shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();

  // alignment[i] = [開始時刻, 終了時刻, phonemes_index 上の index(iになります)]
  std::vector<std::tuple<float, float, int>> alignment;
  solve_viterbi(shape[1], shape[2], y, N, token_ids, alignment);
  if (alignment.empty()) {
    throw std::runtime_error("empty alignment");
  }

  // TODO: 音素をIDから文字列に変換する
  std::vector<std::tuple<double, double, std::string>> labels(alignment.size());
  std::transform(alignment.begin(), alignment.end(), labels.begin(), [&](std::tuple<double, double, int> const &elem) {
    return std::make_tuple(std::get<0>(elem), std::get<1>(elem), token_ids[phonemes_index[std::get<2>(elem)]]);
  });
  return labels；
}

std::vector<int> Aligner::read_phonemes(std::filesystem::path const &file) {
  if (!std::filesystem::is_regular_file(file)) {
    throw std::runtime_error("file is not regular file: " + file.string());
  }
  std::ifstream ss{file};
  return tokenizer.read_phonemes(ss);
}

std::vector<int> Aligner::read_phonemes(std::string const &s) {
  std::istringstream ss{s};
  return tokenizer.read_phonemes(ss);
}
}  // namespace domino
