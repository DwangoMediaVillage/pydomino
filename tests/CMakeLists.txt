cmake_minimum_required(VERSION 3.6...3.14)
project(Testpydomino)
if (WIN32)
    cmake_policy(SET CMP0135 OLD)
endif(WIN32)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

# 依存ライブラリを追加
if (WIN32)
    FetchContent_Declare(onnxruntime URL https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-win-x64-1.16.3.zip)
elseif(APPLE)
    FetchContent_Declare(onnxruntime URL https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-osx-universal2-1.16.3.tgz)
else()
    FetchContent_Declare(onnxruntime URL https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-x64-1.16.3.tgz)
endif()
FetchContent_MakeAvailable(onnxruntime)

include_directories(../eigen ${FETCHCONTENT_BASE_DIR}/onnxruntime-src/include)
link_directories(${FETCHCONTENT_BASE_DIR}/onnxruntime-src/lib)

# テスト時のみ
find_package(HDF5 REQUIRED COMPONENTS C CXX)


enable_testing()

add_executable(
  test_tokenizer
  test_tokenizer.cpp
  ../src/phoneme_transition.cpp
)
target_link_libraries(
  test_tokenizer
  GTest::gtest_main
)

add_executable(
  test_onnxruntime
  test_onnxruntime.cpp
  ../src/load_wav.cpp
)
target_link_libraries(
  test_onnxruntime
  GTest::gtest_main
  onnxruntime
  ${HDF5_CXX_LIBRARIES}
)

add_executable(
  test_viterbi
  test_viterbi.cpp
  ../src/viterbi.cpp
)
target_link_libraries(
  test_viterbi
  GTest::gtest_main
  ${HDF5_CXX_LIBRARIES}
)

# テスト時のみ
find_package(HDF5 REQUIRED COMPONENTS C CXX)
message(STATUS "HDF5_INCLUDE_DIRS= ${HDF5_INCLUDE_DIRS}")
target_include_directories(test_onnxruntime PRIVATE ${HDF5_INCLUDE_DIRS})
target_include_directories(test_viterbi PRIVATE ${HDF5_INCLUDE_DIRS})

include(GoogleTest)
gtest_discover_tests(test_tokenizer)
# gtest_discover_tests(test_onnxruntime)
# gtest_discover_tests(test_viterbi)

