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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif(MSVC)

#依存ライブラリを追加
include(FetchContent)
if (WIN32)
    FetchContent_Declare(onnxruntime URL https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-win-x64-1.16.3.zip)
elseif(APPLE)
    FetchContent_Declare(onnxruntime URL https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-osx-universal2-1.16.3.tgz)
else()
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "AMD" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        FetchContent_Declare(onnxruntime URL https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-aarch64-1.16.3.tgz)
    else()
        FetchContent_Declare(onnxruntime URL https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-x64-1.16.3.tgz)
    endif()
endif()
FetchContent_MakeAvailable(onnxruntime)


add_subdirectory(pybind11)
include_directories(./eigen ./argparse/include ${FETCHCONTENT_BASE_DIR}/onnxruntime-src/include)

pybind11_add_module(
    pydomino_cpp
    src/lib.cpp
    src/domino.cpp
    src/phoneme_transition.cpp
    src/viterbi.cpp
)
file(COPY ${FETCHCONTENT_BASE_DIR} DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
target_link_libraries(
    pydomino_cpp
    PRIVATE onnxruntime
)
if(APPLE)
    set_target_properties(pydomino_cpp PROPERTIES
        INSTALL_RPATH "@loader_path/_deps/onnxruntime-src/lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(pydomino_cpp PROPERTIES
        INSTALL_RPATH "$ORIGIN/_deps/onnxruntime-src/lib/"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()
target_link_directories(pydomino_cpp PRIVATE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/_deps/onnxruntime-src/lib)

add_executable(
    domino
    src/main.cpp
    src/domino.cpp
    src/viterbi.cpp
    src/phoneme_transition.cpp
    src/load_wav.cpp
)
target_link_libraries(
    domino
    PUBLIC onnxruntime
)
if(APPLE)
    set_target_properties(domino PROPERTIES
        INSTALL_RPATH "@loader_path/_deps/onnxruntime-src/lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(domino PROPERTIES
        INSTALL_RPATH "$ORIGIN/_deps/onnxruntime-src/lib/"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()
    
target_link_directories(domino PRIVATE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/_deps/onnxruntime-src/lib)

install(TARGETS domino
    RUNTIME DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

if(WIN32)
    add_custom_command(
        TARGET domino POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${FETCHCONTENT_BASE_DIR}/onnxruntime-src/lib/onnxruntime.dll $<TARGET_FILE_DIR:domino>
    )
    add_custom_command(
        TARGET pydomino_cpp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${FETCHCONTENT_BASE_DIR}/onnxruntime-src/lib/onnxruntime.dll $<TARGET_FILE_DIR:pydomino_cpp>
    )
endif()
