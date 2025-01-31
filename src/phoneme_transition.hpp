#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct PhonemeTransition {
  std::string from_phoneme;
  std::string to_phoneme;

  // unordered_map
  bool operator==(const PhonemeTransition &other) const {
    return from_phoneme == other.from_phoneme && to_phoneme == other.to_phoneme;
  }
};

/**
 * @brief istreamを受け取って内容をパースして対応するトークンID列を出力するクラス
 *
 */

class PhonemeTransitionTokenizer {
 public:
  PhonemeTransitionTokenizer();
  void insert_pause_both_ends_if_not_exists(std::vector<std::string> &input);
  void unique_consecutive(std::vector<std::string> &input);
  void unvoice_i_and_u(std::vector<std::string> &input);
  std::vector<int> read_phonemes(std::istream &ss);
  std::vector<std::string> to_phonemes(std::vector<int> const &token_ids);
  int const get_blank_id();

 private:
  std::vector<PhonemeTransition> id_to_token_map;
  int get_id_from_token(PhonemeTransition const token);
};
