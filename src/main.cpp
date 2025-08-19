#include <argparse/argparse.hpp>
#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>

#include "domino.hpp"
#include "load_wav.hpp"

namespace {
class ElapsedTimer {
 public:
  ElapsedTimer(char const *name) : name_(name), start_(std::chrono::system_clock::now()) {}

  ~ElapsedTimer() {
    std::chrono::system_clock::time_point const end = std::chrono::system_clock::now();
    std::cout << "elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start_).count()
              << " [ms] (" << name_ << ")" << std::endl;
  }

 private:
  char const *const name_;
  std::chrono::system_clock::time_point const start_;
};

std::optional<std::filesystem::path> parse_path(std::optional<std::string> const &s) {
  return s ? std::make_optional(s.value()) : std::nullopt;
}

std::filesystem::path with_suffix(std::filesystem::path const &file, char const *suffix,
                                  std::optional<std::filesystem::path> const &parent_dir = std::nullopt) {
  std::filesystem::path const new_path = std::filesystem::path(file).replace_extension(suffix);
  return parent_dir ? parent_dir.value() / new_path.filename() : new_path;
}

void write_lab_file(std::vector<std::tuple<double, double, std::string>> const &labels,
                    std::filesystem::path const &lab_file) {
  std::cout << "write_lab_file start: \"" << lab_file << "\"" << std::endl;
  std::ofstream lab_ofs(lab_file);
  lab_ofs << std::fixed << std::setprecision(3);
  for (auto const [begin_sec, end_sec, phoneme] : labels) {
    lab_ofs << begin_sec << "\t" << end_sec << "\t" << phoneme << std::endl;
  }
  std::cout << "write_lab_file end: \"" << lab_file << "\"" << std::endl;
}

void write_textGrid_file(std::vector<std::tuple<double, double, std::string>> const &alignment,
                         std::filesystem::path const &textGrid_file) {
  // まずはじめに、開始秒数と終了秒数の等しい要素を削除した alignment を作っておく
  std::vector<std::tuple<double, double, std::string>> non_zero_duration_alignment;
  for (auto const [begin_sec, end_sec, phoneme] : alignment) {
    if (end_sec - begin_sec == 0) {
      continue;
    } else {
      non_zero_duration_alignment.push_back({begin_sec, end_sec, phoneme});
    }
  }

  std::cout << "write_TextGrid_file start: \"" << textGrid_file << "\"" << std::endl;
  std::ofstream textGrid_ofs(textGrid_file);
  textGrid_ofs << std::fixed << std::setprecision(3);

  float total_duration = std::get<1>(non_zero_duration_alignment[non_zero_duration_alignment.size() - 1]);

  textGrid_ofs << "File type = \"ooTextFile\"" << std::endl;
  textGrid_ofs << "Object class = \"TextGrid\"" << std::endl;
  textGrid_ofs << "" << std::endl;

  textGrid_ofs << "xmin = 0" << std::endl;
  textGrid_ofs << "xmax = " << total_duration << std::endl;
  textGrid_ofs << "tiers? <exists>" << std::endl;
  textGrid_ofs << "size = 2" << std::endl;

  textGrid_ofs << "item []:" << std::endl;

  textGrid_ofs << "    item [1]:" << std::endl;
  textGrid_ofs << "        class = \"IntervalTier\"" << std::endl;
  textGrid_ofs << "        name = \"phonemes\"" << std::endl;
  textGrid_ofs << "        xmin = 0" << std::endl;
  textGrid_ofs << "        xmax = " << total_duration << std::endl;
  textGrid_ofs << "        intervals: size = 1" << std::endl;
  textGrid_ofs << "        intervals [1]:" << std::endl;
  textGrid_ofs << "            xmin = 0" << std::endl;
  textGrid_ofs << "            xmax = " << total_duration << std::endl;
  std::string text = std::get<2>(non_zero_duration_alignment[0]);
  for (int i = 1; i < non_zero_duration_alignment.size(); ++i) {
    text += (" " + std::get<2>(non_zero_duration_alignment[i]));
  }
  textGrid_ofs << "            text = \"" << text << "\"" << std::endl;

  textGrid_ofs << "    item [2]:" << std::endl;
  textGrid_ofs << "        class = \"IntervalTier\"" << std::endl;
  textGrid_ofs << "        name = \"alignment result\"" << std::endl;
  textGrid_ofs << "        xmin = 0" << std::endl;
  textGrid_ofs << "        xmax = " << total_duration << std::endl;
  textGrid_ofs << "        intervals: size = " << non_zero_duration_alignment.size() << std::endl;
  int interval_counts = 0;
  for (auto const [begin_sec, end_sec, phoneme] : non_zero_duration_alignment) {
    textGrid_ofs << "        intervals[" << ++interval_counts << "]:" << std::endl;
    textGrid_ofs << "            xmin = " << begin_sec << std::endl;
    textGrid_ofs << "            xmax = " << end_sec << std::endl;
    textGrid_ofs << "            text = \"" << phoneme << "\"" << std::endl;
  }
}
}  // namespace

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("Domino System");
  program.add_argument("--input_path").required().nargs(1).help("wavファイルパスです。");
  program.add_argument("--output_path").nargs(1).help("出力ファイルパスです。");
  program.add_argument("--input_phoneme").nargs(1).help("入力音素列です。音素は半角スペースで区切ってください。");
  program.add_argument("--onnx_path")
      .required()
      .nargs(1)
      .help("onnxファイルパスです。onnx_model/phoneme_transition_model.onnx を推奨します。");
  program.add_argument("--output_format")
      .nargs(1)
      .help("出力ファイルのフォーマット。デフォルトは \"lab\"です。")
      .default_value("lab");
  program.add_argument("--min_frame")
      .nargs(1)
      .help("1音素が割り当てられる最低フレーム数です。デフォルトは 3 です。")
      .default_value(3)
      .scan<'i', int>();

  try {
    ElapsedTimer const total_timer("total");

    program.parse_args(argc, argv);

    {
      domino::Aligner aligner(program.present<std::string>("--onnx_path").value());

      int const N = program.get<int>("--min_frame");

      char const *output_file_ext = [&program]() {
        const auto format = program.get<std::string>("--output_format");
        if (format == "lab") {
          return ".lab";
        } else if (format == "TextGrid") {
          return ".TextGrid";
        }
        throw std::invalid_argument("引数 output_format には、lab か TextGrid のどちらかを指定してください");
      }();

      {
        ElapsedTimer const total_timer("process");

        // ディレクトリパス or ファイルパス のどちらか
        std::filesystem::path const input_path = program.present<std::string>("--input_path").value();
        if (std::filesystem::is_directory(input_path)) {
          std::optional<std::filesystem::path> const output_dir =
              parse_path(program.present<std::string>("--output_path"));
          for (std::filesystem::directory_entry const &file : std::filesystem::directory_iterator{input_path}) {
            std::filesystem::path const wav_file = file.path();
            if (!std::filesystem::is_regular_file(wav_file) || (wav_file.extension() != ".wav")) {
              continue;
            }
            std::string const wav_file_str{wav_file.string()};

            ElapsedTimer const process_timer(wav_file_str.c_str());

            // pythonでいうところの Path.with_suffix()
            std::filesystem::path const txt_file = with_suffix(wav_file, ".txt");
            std::filesystem::path const output_file = with_suffix(wav_file, output_file_ext, output_dir);
            std::filesystem::path const log_file = with_suffix(output_file, ".log");
            std::vector<int> const phonemes_index = aligner.read_phonemes(txt_file);

            std::vector<float> wav_data;
            int load_result = load_wav(wav_file_str.c_str(), wav_data);
            std::cout << "load_wav(" << load_result << "): " << wav_data.size() << std::endl;

            auto const labels = aligner.align(wav_data.data(), wav_data.size(), phonemes_index, N);
            if (program.get<std::string>("--output_format") == "lab") {
              write_lab_file(labels, output_file);
            } else {
              write_textGrid_file(labels, output_file);
            }
          }
        } else if (std::filesystem::is_regular_file(input_path) && input_path.extension() == ".wav") {
          std::filesystem::path const &wav_file = input_path;
          std::string const wav_file_str{wav_file.string()};

          ElapsedTimer const process_timer(wav_file_str.c_str());

          std::filesystem::path const txt_file = with_suffix(wav_file, ".txt");
          std::filesystem::path const log_file = with_suffix(wav_file, ".log");
          std::filesystem::path const output_file = [&program, &wav_file, &output_file_ext]() {
            if (program.present<std::string>("--output_path")) {
              return std::filesystem::path(program.present<std::string>("--output_path").value());
            } else {
              return with_suffix(wav_file, output_file_ext);
            }
          }();
          std::vector<int> const phonemes_index =
              program.present<std::string>("--input_phoneme")
                  ? aligner.read_phonemes(program.present<std::string>("--input_phoneme").value())
                  : aligner.read_phonemes(txt_file);

          std::vector<float> wav_data;
          int load_result = load_wav(wav_file_str.c_str(), wav_data);
          std::cout << "load_wav(" << load_result << "): " << wav_data.size() << std::endl;

          auto const labels = aligner.align(wav_data.data(), wav_data.size(), phonemes_index, N);
          if (program.get<std::string>("--output_format") == "lab") {
            write_lab_file(labels, output_file);
          } else {
            write_textGrid_file(labels, output_file);
          }
        } else {
          // エラー処理
          throw std::runtime_error("invalid input_path: " + input_path.string());
        }
      }
    }
  } catch (Ort::Exception const &e) {
    std::cerr << e.what() << std::endl;
  } catch (std::invalid_argument const &e) {
    std::cerr << e.what() << std::endl;
  } catch (std::logic_error const &e) {
    std::cerr << e.what() << std::endl;
  } catch (std::runtime_error const &e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception const &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}