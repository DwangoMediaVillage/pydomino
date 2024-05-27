/* Pythonライブラリとコマンドコンソール両方のインタフェースを書く場所 */
#include "domino.hpp"

#include <iostream>
#include <sstream>
#include <fstream>

#include <array>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "viterbi.hpp"

namespace {
  class PhonemeMap {
  public:
    std::string operator [] (int const phoneme_id) const {
      if (phoneme_id > phonemes.size()) {
        throw std::runtime_error("その音素IDはmapに用意されていません");
      }
      else {
        return phonemes[phoneme_id];
      }
    }

    int operator [] (std::string const& phoneme) const {
      return phonemes_to_id_map.at(phoneme);
    }

    PhonemeMap() {
      for (int i = 0; i < phonemes.size(); ++i){
        phonemes_to_id_map[phonemes[i]] = i;
      }
    }

  private:
    std::vector<std::string> phonemes = {"0-padding", "ry", "r", "my", "m", "ny", "n", "j", "z", "by", "b", "dy", "d", "gy", "g", "ky", "k", "ch", "ts", "sh", "s", "hy", "h", "v", "f", "py", "p", "t", "y", "w", "N", "a", "i", "u", "e", "o", "I", "U", "cl", "pau"};
    std::unordered_map<std::string, int> phonemes_to_id_map = {};
  } const _phoneme_map;

  void palatalization(std::vector<std::string> &phonemes) {
    for(int i = 0; i < phonemes.size() - 1; ++i) {
      if (!(phonemes[i+1] == "i" || phonemes[i+1] == "I")) continue;
      if (phonemes[i] == "k") {
        phonemes[i] = "ky";
      }else if (phonemes[i] == "s"){
        phonemes[i] = "sh";
      }else if (phonemes[i] == "t") {
        phonemes[i] = "ch";
      }else if (phonemes[i] == "n") {
        phonemes[i] = "ny";
      }else if (phonemes[i] == "h") {
        phonemes[i] = "hy";
      }else if (phonemes[i] == "m") {
        phonemes[i] = "my";
      }else if (phonemes[i] == "r") {
        phonemes[i] = "ry";
      }else if (phonemes[i] == "g") {
        phonemes[i] = "gy";
      }else if (phonemes[i] == "z") {
        phonemes[i] = "j";
      }else if (phonemes[i] == "d") {
        phonemes[i] = "dy";
      }else if (phonemes[i] == "b") {
        phonemes[i] = "by";
      }else if (phonemes[i] == "p") {
        phonemes[i] = "py";
      }
    }
  }

  std::vector<std::tuple<double, double, std::string>> unpalatalization(std::vector<std::tuple<double, double, std::string>> &labels) {
    std::vector<std::tuple<double, double, std::string>> retval;
    for(int i = 0; i < labels.size(); ++i) {
      if (i == labels.size() - 1) {
        retval.push_back(labels[i]);
        continue;
      }

      if (!(std::get<2>(labels[i+1])  == "i" || std::get<2>(labels[i+1]) == "I")) {
        retval.push_back(labels[i]);
        continue;
      }
      std::string phoneme = std::get<2>(labels[i]);
      if (phoneme == "ky") {
        retval.push_back(std::make_tuple(std::get<0>(labels[i]), std::get<1>(labels[i]), "k"));
      }else if (phoneme == "sh"){
        retval.push_back(std::make_tuple(std::get<0>(labels[i]), std::get<1>(labels[i]), "s"));
      }else if (phoneme == "ch") {
        retval.push_back(std::make_tuple(std::get<0>(labels[i]), std::get<1>(labels[i]), "t"));
      }else if (phoneme == "ny") {
        retval.push_back(std::make_tuple(std::get<0>(labels[i]), std::get<1>(labels[i]), "n"));
      }else if (phoneme == "hy") {
        retval.push_back(std::make_tuple(std::get<0>(labels[i]), std::get<1>(labels[i]), "h"));
      }else if (phoneme == "my") {
        retval.push_back(std::make_tuple(std::get<0>(labels[i]), std::get<1>(labels[i]), "m"));
      }else if (phoneme == "ry") {
        retval.push_back(std::make_tuple(std::get<0>(labels[i]), std::get<1>(labels[i]), "r"));
      }else if (phoneme == "gy") {
        retval.push_back(std::make_tuple(std::get<0>(labels[i]), std::get<1>(labels[i]), "g"));
      }else if (phoneme == "j") {
        retval.push_back(std::make_tuple(std::get<0>(labels[i]), std::get<1>(labels[i]), "z"));
      }else if (phoneme == "dy") {
        retval.push_back(std::make_tuple(std::get<0>(labels[i]), std::get<1>(labels[i]), "d"));
      }else if (phoneme == "by") {
        retval.push_back(std::make_tuple(std::get<0>(labels[i]), std::get<1>(labels[i]), "b"));
      }else if (phoneme == "py") {
        retval.push_back(std::make_tuple(std::get<0>(labels[i]), std::get<1>(labels[i]), "p"));
      }else {
        retval.push_back(labels[i]);
      }
    }
    return retval;
  }

  std::vector<int> read_phonemes(std::istream& ss) {
    std::vector<std::string> phonemes_str;
    std::vector<int> phonemes_index;
    {
      std::string buf;
      while (ss && !ss.eof()) {
        ss >> buf;
        phonemes_str.push_back(buf);
      }
      palatalization(phonemes_str);

      for(auto const &phoneme: phonemes_str) {
        phonemes_index.push_back(_phoneme_map[phoneme]);
      }
    }
    return phonemes_index;
  }

}

namespace domino {
  Aligner::Aligner(std::string const& path, int const N)
    : env_()
    , session_options_()
    , session_(env_, std::filesystem::path(path).c_str(), session_options_)
    , memory_info_(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU))
    , run_options_()
    , N_(N)
  {
    std::cout << "path: " << path << std::endl;
  }

  Aligner::~Aligner() {
    this->release();
  }

  void Aligner::release() {
    run_options_.release();
    memory_info_.release();
    session_.release();
    env_.release();
  }

  std::vector<std::tuple<double, double, std::string>> Aligner::align_phonemes(Eigen::Ref<Eigen::VectorXf> const wav_data, std::string const& phonemes, int N)
  {
    return this->align(wav_data.data(), wav_data.size(), Aligner::read_phonemes(phonemes), N);
  }

  std::vector<std::tuple<double, double, std::string>> Aligner::align(float const* wav_data, std::size_t const wav_data_size, std::vector<int> const& phonemes_index, int N)
  {
    if (N < 1) {
        N = N_;
    }

    constexpr char const* const input_names[] = { "x" };
    constexpr char const* const output_names[] = { "y" };
    // NOTE: C++17以上が必須
    std::array<std::int64_t, 2> const wav_data_shape = { 1, static_cast<std::int64_t>(wav_data_size) };
    Ort::Value inputs[] = {
        Ort::Value::CreateTensor(memory_info_,
                                 const_cast<float*>(wav_data), wav_data_size,
                                 wav_data_shape.data(), wav_data_shape.size()),
    };
    std::vector<Ort::Value> const outputs = session_.Run(
        run_options_,
        input_names, inputs, std::size(input_names),
        output_names, std::size(output_names)
    );
    // output y.shape = (1, len_time_frame, num_kind_phonemes)
    float const* const y = outputs[0].GetTensorData<float>();
    auto const shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();

    // alignment[i] = [開始時刻, 終了時刻, phonemes_index 上の index (iになります)]
    std::vector<std::tuple<float, float, int>> alignment;
    solve_viterbi(shape[1], shape[2], y, N, phonemes_index, alignment);
    if (alignment.empty()) {
        throw std::runtime_error("empty alignment");
    }
   
    // TODO 音素をIDから文字列に変換する
    std::vector<std::tuple<double, double, std::string>> labels(alignment.size());
    std::transform(alignment.begin(), alignment.end(),
                   labels.begin(),
                   [&](std::tuple<double, double, int> const& elem) {
                     return std::make_tuple(std::get<0>(elem), std::get<1>(elem), _phoneme_map[phonemes_index[std::get<2>(elem)]]);
                   });
    return unpalatalization(labels);
  }

  std::vector<int> Aligner::read_phonemes(std::filesystem::path const& file) {
    if (!std::filesystem::is_regular_file(file)) {
      throw std::runtime_error("file is not regular file: " + file.string());
    }
    std::ifstream ss{file};
    return ::read_phonemes(ss);
  }

  std::vector<int> Aligner::read_phonemes(std::string const& s) {
    std::istringstream ss{s};
    return ::read_phonemes(ss);
  }
}
