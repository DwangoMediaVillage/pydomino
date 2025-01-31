#include <gtest/gtest.h>

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "../src/phoneme_transition.hpp"

TEST(Test_PhonemeTransitionTokenizer, test_insert_pause_both_ends_if_not_exists) {
  std::filesystem::current_path("../../");
  PhonemeTransitionTokenizer tokenizer = PhonemeTransitionTokenizer();
  std::vector<std::string> input_not_both_edge = {"d", "o", "w", "a", "N", "g", "o"};
  std::vector<std::string> expected_output = {"pau", "d", "o", "w", "a", "N", "g", "o", "pau"};
  tokenizer.insert_pause_both_ends_if_not_exists(input_not_both_edge);
  EXPECT_EQ(input_not_both_edge, expected_output);

  std::vector<std::string> input_only_first_pau = {"pau", "d", "o", "w", "a", "N", "g", "o"};
  tokenizer.insert_pause_both_ends_if_not_exists(input_only_first_pau);
  EXPECT_EQ(input_only_first_pau, expected_output);

  std::vector<std::string> input_only_last_pau = {"d", "o", "w", "a", "N", "g", "o", "pau"};
  tokenizer.insert_pause_both_ends_if_not_exists(input_only_last_pau);
  EXPECT_EQ(input_only_last_pau, expected_output);

  std::vector<std::string> input_exist_both_edge = {"pau", "d", "o", "w", "a", "N", "g", "o", "pau"};
  tokenizer.insert_pause_both_ends_if_not_exists(input_exist_both_edge);
  EXPECT_EQ(input_exist_both_edge, expected_output);
}

TEST(Test_PhonemeTransitionTokenizer, test_unvoice_i_and_u) {
  PhonemeTransitionTokenizer tokenizer = PhonemeTransitionTokenizer();
  std::vector<std::string> input_tasuuketsu = {"pau", "t", "a", "s", "u", "u", "k", "e", "ts", "u", "pau"};
  std::vector<std::string> expected_output_tasuuketsu = {"pau", "t", "a", "s", "u", "u", "k", "e", "ts", "U", "pau"};
  tokenizer.unvoice_i_and_u(input_tasuuketsu);
  EXPECT_EQ(input_tasuuketsu, expected_output_tasuuketsu);

  std::vector<std::string> input_ishiki = {"pau", "i", "sh", "i", "k", "i", "pau"};
  std::vector<std::string> expected_output_ishiki = {"pau", "i", "sh", "I", "k", "I", "pau"};
  tokenizer.unvoice_i_and_u(input_ishiki);
  EXPECT_EQ(input_ishiki, expected_output_ishiki);
}

void test_unique_consecitive_func(std::vector<std::string> input, std::vector<std::string> expected) {
  PhonemeTransitionTokenizer tokenizer = PhonemeTransitionTokenizer();
  tokenizer.unique_consecutive(input);
  EXPECT_EQ(input, expected);
}

TEST(Test_PhonemeTransitionTokenizer, test_unique_consecitive) {
  test_unique_consecitive_func({"pau", "t", "a", "s", "u", "u", "k", "e", "ts", "u", "pau"},
                               {"pau", "t", "a", "s", "u", "k", "e", "ts", "u", "pau"});
  test_unique_consecitive_func({"h", "o", "o", "o", "o", "i", "n", "ky", "o", "o", "m", "a"},
                               {"h", "o", "i", "n", "ky", "o", "m", "a"});
}

void test_read_phonemes_func(std::string input, std::vector<int> expected) {
  PhonemeTransitionTokenizer tokenizer = PhonemeTransitionTokenizer();
  std::istringstream stream{input};
  std::vector<int> token_ids = tokenizer.read_phonemes(stream);
  EXPECT_EQ(token_ids, expected);
}

TEST(Test_PhonemeTransitionTokenizer, test_read_phonemes) {
  test_read_phonemes_func("pau t a s u u k e ts u pau", {546, 182, 222, 135, 292, 108, 331, 125, 447});
  test_read_phonemes_func("pau i sh i k i pau", {550, 258, 131, 403, 110, 417});
}

void test_to_phonemes_func(std::string input, std::vector<std::string> expected) {
  PhonemeTransitionTokenizer tokenizer = PhonemeTransitionTokenizer();
  std::istringstream stream{input};
  std::vector<int> const token_ids = tokenizer.read_phonemes(stream);
  std::vector<std::string> const output_phonemes = tokenizer.to_phonemes(token_ids);
  EXPECT_EQ(output_phonemes, expected);
}

TEST(Test_PhonemeTransitionTokenizer, test_to_phonemes) {
  test_to_phonemes_func("d o w a N g o", {"pau", "d", "o", "w", "a", "N", "g", "o", "pau"});
  test_to_phonemes_func("pau t a s u u k e ts u pau", {"pau", "t", "a", "s", "u", "k", "e", "ts", "U", "pau"});
  test_to_phonemes_func("i sh i k i pau", {"pau", "i", "sh", "I", "k", "I", "pau"});
}