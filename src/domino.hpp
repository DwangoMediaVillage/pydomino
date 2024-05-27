#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <tuple>

#include <onnxruntime_cxx_api.h>
#include <Eigen/Core>

namespace domino {
  class Aligner {
  public:
    Aligner(std::string const& path, int const N = 5);
    ~Aligner();

    void release();
    
    // std::vector<std::tuple<double, double, std::string>>: labデータの構造
    std::vector<std::tuple<double, double, std::string>> align_phonemes(Eigen::Ref<Eigen::VectorXf> const wav, std::string const& phonemes, int N = 0);
    std::vector<std::tuple<double, double, std::string>> align(float const* wav_data, std::size_t const wav_data_size, std::vector<int> const& phonemes_index, int N = 0);

    static std::vector<int> read_phonemes(std::filesystem::path const& file);
    static std::vector<int> read_phonemes(std::string const& s);

  private:
    Ort::Env env_;
    Ort::SessionOptions session_options_;
    Ort::Session session_;
    Ort::MemoryInfo memory_info_;
    Ort::RunOptions run_options_;

    int const N_;
  };
}
