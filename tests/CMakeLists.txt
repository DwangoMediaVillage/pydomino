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


enable_testing()

add_executable(
  test_domino
  test_domino.cpp
  ../src/phoneme_transition.cpp
)
target_link_libraries(
  test_domino
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_domino)

