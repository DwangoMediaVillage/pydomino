cmake_minimum_required(VERSION 3.6...3.14)
project(domino)
if (WIN32)
    cmake_policy(SET CMP0135 OLD)
endif(WIN32)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
else(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -lpthread")
endif(MSVC)

# 依存ライブラリを追加
include(FetchContent)
if (WIN32)
    FetchContent_Declare(onnxruntime URL https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-win-x64-1.16.3.zip)
elseif(APPLE)
    FetchContent_Declare(onnxruntime URL https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-osx-universal2-1.16.3.tgz)
else()
    FetchContent_Declare(onnxruntime URL https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-x64-1.16.3.tgz)
endif()
FetchContent_MakeAvailable(onnxruntime)

add_subdirectory(pybind11)
include_directories(./eigen ./argparse/include ${FETCHCONTENT_BASE_DIR}/onnxruntime-src/include)
link_directories(${FETCHCONTENT_BASE_DIR}/onnxruntime-src/lib)

pybind11_add_module(
    pydomino
    src/lib.cpp
    src/domino.cpp
    src/viterbi.cpp
)
target_link_libraries(
    pydomino
    PRIVATE onnxruntime
)

add_executable(
    domino
    src/main.cpp
    src/domino.cpp
    src/viterbi.cpp
    src/load_wav.cpp
)
target_link_libraries(
    domino
    PRIVATE onnxruntime
)

if(WIN32)
    add_custom_command(
        TARGET pydomino POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${FETCHCONTENT_BASE_DIR}/onnxruntime-src/lib/onnxruntime.dll $<TARGET_FILE_DIR:pydomino>
    )
endif()
