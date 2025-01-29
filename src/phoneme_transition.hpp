#pragma once

#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief istreamを受け取って内容をパースして対応するトークンID列を出力するクラス
 *
 */

class PhonemeTransitionTokenizer {
 public:
  PhonemeTransitionTokenizer();
  std::string operator[](int const token_id) const;
  int operator[](std::string const &token) const;
  void insert_pause_both_ends_if_not_exists(std::vector<std::string> &input);
  void unique_consecutive(std::vector<std::string> &input);
  void unvoice_i_and_u(std::vector<std::string> &input);
  std::vector<int> read_phonemes(std::istream &ss);

 private:
  std::unordered_map<std::string, int> token_to_id_map;
  std::vector<std::string> id_to_token_map;
};
