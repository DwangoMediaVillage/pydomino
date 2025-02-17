#include <H5Cpp.h>
#include <gtest/gtest.h>
#include <onnxruntime_cxx_api.h>

#include <filesystem>
#include <string>
#include <tuple>
#include <vector>

#include "../src/load_wav.hpp"

void saveFloatArrayHDF5(float const *const data, size_t rows, size_t cols, const std::string &filename) {
  H5::H5File file(filename, H5F_ACC_TRUNC);
  hsize_t dims[2] = {rows, cols};
  H5::DataSpace dataspace(2, dims);
  H5::DataSet dataset = file.createDataSet("array", H5::PredType::NATIVE_FLOAT, dataspace);
  dataset.write(data, H5::PredType::NATIVE_FLOAT);
}

void saveFloatArrayHDF5(float const *const data, size_t rows, const std::string &filename) {
  H5::H5File file(filename, H5F_ACC_TRUNC);
  hsize_t dims[1] = {rows};
  H5::DataSpace dataspace(1, dims);
  H5::DataSet dataset = file.createDataSet("array", H5::PredType::NATIVE_FLOAT, dataspace);
  dataset.write(data, H5::PredType::NATIVE_FLOAT);
}

class Network {
 public:
  Network()
      : env_(),
        session_options_(),
        session_(env_, "onnx_model/onnx_model/phoneme_trantision_model.onnx", session_options_),
        memory_info_(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)),
        run_options_() {
    ;
  }

  void run(char const *mono_16kHz_16bit_wav_file, std::string const &output_transition_hdf5_file,
           std::string const &output_blank_hdf5_file) {
    std::vector<float> wav_data;
    load_wav(mono_16kHz_16bit_wav_file, wav_data);
    constexpr char const *const input_names[] = {"input_waveform"};
    constexpr char const *const output_names[] = {"transition_logprobs", "blank_logprobs"};
    // NOTE: C++17以上が必須

    std::array<std::int64_t, 2> const wav_data_shape = {1, static_cast<std::int64_t>(wav_data.size())};
    Ort::Value inputs[] = {
        Ort::Value::CreateTensor(memory_info_, const_cast<float *>(wav_data.data()), wav_data.size(),
                                 wav_data_shape.data(), wav_data_shape.size()),
    };
    std::vector<Ort::Value> const outputs =
        session_.Run(run_options_, input_names, inputs, std::size(input_names), output_names, std::size(output_names));
    float const *const transition_logprobs = outputs[0].GetTensorData<float>();
    auto const transition_logprobs_shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
    saveFloatArrayHDF5(transition_logprobs, transition_logprobs_shape[1], transition_logprobs_shape[2],
                       output_transition_hdf5_file);
    float const *const blank_logprobs = outputs[1].GetTensorData<float>();
    auto const blank_logprobs_shape = outputs[1].GetTensorTypeAndShapeInfo().GetShape();
    saveFloatArrayHDF5(blank_logprobs, blank_logprobs_shape[1], output_blank_hdf5_file);
  }

 private:
  Ort::Env env_;
  Ort::SessionOptions session_options_;
  Ort::Session session_;
  Ort::MemoryInfo memory_info_;
  Ort::RunOptions run_options_;
};

TEST(Test_ONNXModel, test_can_work) {
  std::filesystem::current_path("../../");
  Network n = Network();

  n.run("tests/wavdata/dowaNgo.wav", "tests/prediction_logprobs/dowaNgo_transition_logprobs.h5",
        "tests/prediction_logprobs/dowaNgo_blank_logprobs.h5");
  n.run("tests/wavdata/tasuuketsU.wav", "tests/prediction_logprobs/tasuuketsU_transition_logprobs.h5",
        "tests/prediction_logprobs/tasuuketsU_blank_logprobs.h5");
  n.run("tests/wavdata/ishIkI.wav", "tests/prediction_logprobs/ishIkI_transition_logprobs.h5",
        "tests/prediction_logprobs/ishIkI_blank_logprobs.h5");
}