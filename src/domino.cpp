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

namespace
{
  // _ 区切りで遷移を表現しよう
  class PhonemeTransitionMap
  {
  public:
    std::string operator[](int const phoneme_id) const
    {
      if (phoneme_id > id_to_token_map.size())
      {
        throw std::runtime_error("その音素IDはmapに用意されていません");
      }
      else
      {
        return id_to_token_map[phoneme_id];
      }
    }

    int operator[](std::string const &phoneme) const
    {
      return token_to_id_map.at(phoneme);
    }

    /**
     * @brief Construct a new Phoneme Transition Map object
     * |先\後|pau|子音|有声母音|無声母音|N|cl|
     * |--|--|--|--|--|--|--|
     * |pau|x|o|o|x|o|o|
     * |子音|x|x|o|o|x|x|
     * |有声母音|o|o|o|x|o|o|
     * |無声母音|o|o|x|x|x|x|
     * |N|o|o|o|x|x|o|
     * |cl|o|o|o|x|o|x|
     */
    PhonemeTransitionMap()
    {
      int token_id = 0;
      token_to_id_map[blank] = token_id++;
      id_to_token_map.push_back(blank);

      std::vector<std::string> phonemes_next_to_pause = {};
      phonemes_next_to_pause.insert(phonemes_next_to_pause.end(), consonants.begin(), consonants.end());
      phonemes_next_to_pause.insert(phonemes_next_to_pause.end(), voice_vowels.begin(), voice_vowels.end());
      phonemes_next_to_pause.insert(phonemes_next_to_pause.end(), syllabic_nasal);
      phonemes_next_to_pause.insert(phonemes_next_to_pause.end(), double_consonant);
      for (auto const &phoneme : phonemes_next_to_pause)
      {
        std::string transition = pause + "_" + phoneme;
        token_to_id_map[transition] = token_id++;
        id_to_token_map.push_back(transition);
      }

      std::vector<std::string> phonemes_next_to_consonants = {};
      phonemes_next_to_consonants.insert(phonemes_next_to_consonants.end(), voice_vowels.begin(), voice_vowels.end());
      phonemes_next_to_consonants.insert(phonemes_next_to_consonants.end(), voiceless_vowels.begin(), voiceless_vowels.end());
      for (auto const &consonant : consonants)
      {
        for (auto const &phoneme : phonemes_next_to_consonants)
        {
          std::string transition = consonant + "_" + phoneme;
          token_to_id_map[transition] = token_id++;
          id_to_token_map.push_back(transition);
        }
      }

      std::vector<std::string> phonemes_next_to_voice_vowels = {};
      phonemes_next_to_voice_vowels.insert(phonemes_next_to_voice_vowels.end(), pause);
      phonemes_next_to_voice_vowels.insert(phonemes_next_to_voice_vowels.end(), consonants.begin(), consonants.end());
      phonemes_next_to_voice_vowels.insert(phonemes_next_to_voice_vowels.end(), voice_vowels.begin(), voice_vowels.end());
      phonemes_next_to_voice_vowels.insert(phonemes_next_to_voice_vowels.end(), syllabic_nasal);
      phonemes_next_to_voice_vowels.insert(phonemes_next_to_voice_vowels.end(), double_consonant);
      for (auto const &vowel : voice_vowels)
      {
        for (auto const &phoneme : phonemes_next_to_voice_vowels)
        {
          std::string transition = vowel + "_" + phoneme;
          token_to_id_map[transition] = token_id++;
          id_to_token_map.push_back(transition);
        }
      }

      std::vector<std::string> phonemes_next_to_voiceless_vowels = {};
      phonemes_next_to_voiceless_vowels.insert(phonemes_next_to_voiceless_vowels.end(), pause);
      phonemes_next_to_voiceless_vowels.insert(phonemes_next_to_voiceless_vowels.end(), consonants.begin(), consonants.end());
      for (auto const &vowel : voiceless_vowels)
      {
        for (auto const &phoneme : phonemes_next_to_voiceless_vowels)
        {
          std::string transition = vowel + "_" + phoneme;
          token_to_id_map[transition] = token_id++;
          id_to_token_map.push_back(transition);
        }
      }

      std::vector<std::string> phonemes_next_to_syllabic_nasal = {};
      phonemes_next_to_syllabic_nasal.insert(phonemes_next_to_syllabic_nasal.end(), pause);
      phonemes_next_to_syllabic_nasal.insert(phonemes_next_to_syllabic_nasal.end(), consonants.begin(), consonants.end());
      phonemes_next_to_syllabic_nasal.insert(phonemes_next_to_syllabic_nasal.end(), voice_vowels.begin(), voice_vowels.end());
      phonemes_next_to_syllabic_nasal.insert(phonemes_next_to_syllabic_nasal.end(), double_consonant);
      for (auto const &phoneme : phonemes_next_to_syllabic_nasal)
      {
        std::string transition = syllabic_nasal + "_" + phoneme;
        token_to_id_map[transition] = token_id++;
        id_to_token_map.push_back(transition);
      }

      std::vector<std::string> phonemes_next_to_double_consonant = {};
      phonemes_next_to_double_consonant.insert(phonemes_next_to_double_consonant.end(), pause);
      phonemes_next_to_double_consonant.insert(phonemes_next_to_double_consonant.end(), consonants.begin(), consonants.end());
      phonemes_next_to_double_consonant.insert(phonemes_next_to_double_consonant.end(), voice_vowels.begin(), voice_vowels.end());
      phonemes_next_to_double_consonant.insert(phonemes_next_to_double_consonant.end(), syllabic_nasal);
      for (auto const &phoneme : phonemes_next_to_double_consonant)
      {
        std::string transition = double_consonant + "_" + phoneme;
        token_to_id_map[transition] = token_id++;
        id_to_token_map.push_back(transition);
      }
    }

  private:
    std::vector<std::string> const consonants = {"ry", "r", "my", "m", "ny", "n", "j", "z", "by", "b", "dy", "d", "gy", "g", "ky", "k", "ch", "ts", "sh", "s", "hy", "h", "v", "f", "py", "p", "t", "y", "w"};
    std::vector<std::string> const voice_vowels = {"a", "i", "u", "e", "o"};
    std::vector<std::string> const voiceless_vowels = {"I", "U"};
    std::string const syllabic_nasal = "N";
    std::string const double_consonant = "cl";
    std::string const pause = "pau";
    std::string const blank = "blank";
    std::unordered_map<std::string, int> token_to_id_map = {};
    std::vector<std::string> id_to_token_map = {};
  } const _phoneme_transition_map;

  std::vector<int> read_phonemes(std::istream &ss)
  {
    std::vector<std::string> phonemes_str;
    std::vector<int> phonemes_index;
    {
      std::string buf;
      while (ss && !ss.eof())
      {
        ss >> buf;
        phonemes_str.push_back(buf);
      }
      palatalization(phonemes_str);

      for (auto const &phoneme : phonemes_str)
      {
        phonemes_index.push_back(_phoneme_map[phoneme]);
      }
    }
    return phonemes_index;
  }

}

namespace domino
{
  Aligner::Aligner(std::string const &path, int const N)
      : env_(), session_options_(), session_(env_, std::filesystem::path(path).c_str(), session_options_), memory_info_(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)), run_options_(), N_(N)
  {
    std::cout << "path: " << path << std::endl;
  }

  Aligner::~Aligner()
  {
    this->release();
  }

  void Aligner::release()
  {
    run_options_.release();
    memory_info_.release();
    session_.release();
    env_.release();
  }

  std::vector<std::tuple<double, double, std::string>> Aligner::align_phonemes(Eigen::Ref<Eigen::VectorXf> const wav_data, std::string const &phonemes, int N)
  {
    return this->align(wav_data.data(), wav_data.size(), Aligner::read_phonemes(phonemes), N);
  }

  std::vector<std::tuple<double, double, std::string>> Aligner::align(float const *wav_data, std::size_t const wav_data_size, std::vector<int> const &phonemes_index, int N)
  {
    if (N < 1)
    {
      N = N_;
    }

    constexpr char const *const input_names[] = {"x"};
    constexpr char const *const output_names[] = {"y"};
    // NOTE: C++17以上が必須
    std::array<std::int64_t, 2> const wav_data_shape = {1, static_cast<std::int64_t>(wav_data_size)};
    Ort::Value inputs[] = {
        Ort::Value::CreateTensor(memory_info_,
                                 const_cast<float *>(wav_data), wav_data_size,
                                 wav_data_shape.data(), wav_data_shape.size()),
    };
    std::vector<Ort::Value> const outputs = session_.Run(
        run_options_,
        input_names, inputs, std::size(input_names),
        output_names, std::size(output_names));
    // output y.shape = (1, len_time_frame, num_kind_phonemes)
    float const *const y = outputs[0].GetTensorData<float>();
    auto const shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();

    // alignment[i] = [開始時刻, 終了時刻, phonemes_index 上の index (iになります)]
    std::vector<std::tuple<float, float, int>> alignment;
    solve_viterbi(shape[1], shape[2], y, N, phonemes_index, alignment);
    if (alignment.empty())
    {
      throw std::runtime_error("empty alignment");
    }

    // TODO 音素をIDから文字列に変換する
    std::vector<std::tuple<double, double, std::string>> labels(alignment.size());
    std::transform(alignment.begin(), alignment.end(),
                   labels.begin(),
                   [&](std::tuple<double, double, int> const &elem)
                   {
                     return std::make_tuple(std::get<0>(elem), std::get<1>(elem), _phoneme_map[phonemes_index[std::get<2>(elem)]]);
                   });
    return unpalatalization(labels);
  }

  std::vector<int> Aligner::read_phonemes(std::filesystem::path const &file)
  {
    if (!std::filesystem::is_regular_file(file))
    {
      throw std::runtime_error("file is not regular file: " + file.string());
    }
    std::ifstream ss{file};
    return ::read_phonemes(ss);
  }

  std::vector<int> Aligner::read_phonemes(std::string const &s)
  {
    std::istringstream ss{s};
    return ::read_phonemes(ss);
  }
}
