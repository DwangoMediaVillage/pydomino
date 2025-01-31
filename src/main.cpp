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
  lab_ofs << std::fixed << std::setprecision(2);
  for (auto const [begin_sec, end_sec, phoneme] : labels) {
    lab_ofs << begin_sec << "\t" << end_sec << "\t" << phoneme << std::endl;
  }
  std::cout << "write_lab_file end: \"" << lab_file << "\"" << std::endl;
}
}  // namespace

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("Domino System");
  program.add_argument("--input_path").required().nargs(1).help("wavファイルパス");
  program.add_argument("--output_path").nargs(1).help("labファイルパス");
  program.add_argument("--input_phoneme").nargs(1).help("入力音素列");
  program.add_argument("-N").nargs(1).help("1音素が割り当てられる最低フレーム数").default_value(5).scan<'i', int>();

  try {
    ElapsedTimer const total_timer("total");

    program.parse_args(argc, argv);

    {
      domino::Aligner aligner("onnx_model/phoneme_trantision_model_2.onnx");

      int const N = program.get<int>("-N");

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
            std::filesystem::path const lab_file = with_suffix(wav_file, ".lab", output_dir);
            std::filesystem::path const log_file = with_suffix(lab_file, ".log");
            std::vector<int> const phonemes_index = aligner.read_phonemes(txt_file);

            std::vector<float> wav_data;
            int load_result = load_wav(wav_file_str.c_str(), wav_data);
            std::cout << "load_wav(" << load_result << "): " << wav_data.size() << std::endl;

            auto const labels = aligner.align(wav_data.data(), wav_data.size(), phonemes_index, N);
            write_lab_file(labels, lab_file);
          }
        } else if (std::filesystem::is_regular_file(input_path) && input_path.extension() == ".wav") {
          std::filesystem::path const &wav_file = input_path;
          std::string const wav_file_str{wav_file.string()};

          ElapsedTimer const process_timer(wav_file_str.c_str());

          std::filesystem::path const txt_file = with_suffix(wav_file, ".txt");
          std::filesystem::path const lab_file =
              program.present<std::string>("--output_path")
                  ? std::filesystem::path(program.present<std::string>("--output_path").value())
                  : with_suffix(wav_file, ".lab");
          std::filesystem::path const log_file = with_suffix(lab_file, ".log");

          std::vector<int> const phonemes_index =
              program.present<std::string>("--input_phoneme")
                  ? aligner.read_phonemes(program.present<std::string>("--input_phoneme").value())
                  : aligner.read_phonemes(txt_file);

          std::vector<float> wav_data;
          int load_result = load_wav(wav_file_str.c_str(), wav_data);
          std::cout << "load_wav(" << load_result << "): " << wav_data.size() << std::endl;

          auto const labels = aligner.align(wav_data.data(), wav_data.size(), phonemes_index, N);
          write_lab_file(labels, lab_file);
        } else {
          // エラー処理
          throw std::runtime_error("invalid input_path: " + input_path.string());
        }
      }
    }
  } catch (Ort::Exception const &e) {
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
