#include "phoneme_transition.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

/**
 * @brief istreamを受け取って内容をパースして対応するトークンID列を出力するクラス
 *
 */

/**
 * @brief 内部で phoneme_transition.txt で全音素遷移トークンリストを読み込む
 *
 */
PhonemeTransitionTokenizer::PhonemeTransitionTokenizer() {
  std::ifstream tokenlist_file("src/phoneme_transitions.txt");
  if (!tokenlist_file) {
    throw std::runtime_error("src/phoneme_transitions.txt cannot be opened.");
  }

  std::string transition_token;
  int token_id = 0;
  id_to_token_map.reserve(558);  // 音素遷移の総数
  while (std::getline(tokenlist_file, transition_token)) {
    token_to_id_map[transition_token] = token_id++;
    id_to_token_map.push_back(transition_token);
  }

  tokenlist_file.close();
}

/**
 * @brief 入力した音素列の両端がpau (無音; pause) トークン でない場合に、両端がpauになるよう挿入する関数
 *
 * @param phonemes 入力音素列
 */
void PhonemeTransitionTokenizer::insert_pause_both_ends_if_not_exists(std::vector<std::string> &phonemes) {
  if (phonemes[0] != "pau") {
    phonemes.insert(phonemes.begin(), "pau");
  }
  if (phonemes[phonemes.size() - 1] != "pau") {
    phonemes.insert(phonemes.end(), "pau");
  }
}

/**
 * @brief 入力した音素列の "i", "u" が無声母音になる条件を満たすときに、無声母音"I", "U" にそれぞれ変換する関数
 *
 * @param phonemes 入力音素列
 */
void PhonemeTransitionTokenizer::unvoice_i_and_u(std::vector<std::string> &phonemes) {
  std::vector<std::string> const devoicing_phoneme = {
      "k", "ky", "ch", "ts", "sh", "s", "hy", "h", "f", "py", "p", "t",
  };

  std::vector<std::string> const devoicing_phoneme_with_pau = {
      "k", "ky", "ch", "ts", "sh", "s", "hy", "h", "f", "py", "p", "t", "pau",
  };

  for (int i = 1; i < phonemes.size(); ++i) {
    if (phonemes[i] == "i" || phonemes[i] == "u") {
      if (std::find(devoicing_phoneme.begin(), devoicing_phoneme.end(), phonemes[i - 1]) != devoicing_phoneme.end() &&
          std::find(devoicing_phoneme_with_pau.begin(), devoicing_phoneme_with_pau.end(), phonemes[i + 1]) !=
              devoicing_phoneme_with_pau.end()) {
        if (phonemes[i] == "i") {
          phonemes[i] = "I";
        } else {
          phonemes[i] = "U";
        }
      }
    }
  }
}

/**
 * @brief 伸ばし音などで同じ音素が連続で入力されたときに、それを1つにまとめる関数。名前の由来は torch.unique_consecutive
 * から
 *
 * @param phonemes 入力音素列
 */
void PhonemeTransitionTokenizer::unique_consecutive(std::vector<std::string> &phonemes) {
  std::vector<std::string> results;
  results.reserve(phonemes.size());

  for (int i = 0; i < phonemes.size() - 1; ++i) {
    if (phonemes[i] == phonemes[i + 1]) {
      continue;
    }
    results.push_back(phonemes[i]);
  }
  results.push_back(phonemes[phonemes.size() - 1]);
  phonemes.swap(results);
}

std::vector<int> PhonemeTransitionTokenizer::read_phonemes(std::istream &ss) {
  std::vector<std::string> phonemes;
  std::vector<int> phonemes_index;
  {
    std::string buf;
    while (ss && !ss.eof()) {
      ss >> buf;
      phonemes.push_back(buf);
    }
    insert_pause_both_ends_if_not_exists(phonemes);
    unvoice_i_and_u(phonemes);
    unique_consecutive(phonemes);

    for (int i = 1; i < phonemes.size(); ++i) {
      std::cout << phonemes[i - 1] + "->" + phonemes[i] << std::endl;
      phonemes_index.push_back(token_to_id_map[phonemes[i - 1] + "->" + phonemes[i]]);
    }
  }
  return phonemes_index;
}
