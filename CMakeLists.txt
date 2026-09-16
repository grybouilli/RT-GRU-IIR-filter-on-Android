cmake_minimum_required(VERSION 3.24)  
project(filtered LANGUAGES C CXX)  
set(CMAKE_CXX_STANDARD 20)            

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
set(SOURCE_FILES src/main.cpp src/acquire_audio_stream.cpp src/utils.cpp)
set(GET_OPT_MODEL_SRC_FILES src/get_optimized_model.cpp src/utils.cpp)
set(OFFLINE_INFERENCE_SRC_FILES src/offline_inferences.cpp src/utils.cpp)

set(GET_OPT_MODEL_BIN get_opt_model)
set(OFFLINE_INFERENCE_3SEC offline_inference_3sec)
set(OFFLINE_INFERENCE_2SEC offline_inference_2sec)

option(ONNX_USE_QNN "Will fetch a version of Onnx Runtime that only supports QNN and CPU" OFF)
option(APP_AS_APK "Will build app as a complete Android APK app" OFF)

set(ANIRA_WITH_TFLITE OFF)
set(ANIRA_WITH_LIBTORCH OFF)
set(LIBONNXRUNTIME_VERSION 1.29.0) # minimum required is 1.28.0 to get PR #28733 and 1.29.0 to get PR #30512

include(cmake/DynamicONNXRuntime.cmake)
add_subdirectory(modules/anira)
include(cmake/liboboe.cmake)
include(cmake/cxxopts.cmake)
include(cmake/magic_enum.cmake)
include(cmake/nlohmann_json.cmake)
include(cmake/boost_pfr.cmake)
include(cmake/FetchLibsamplerate.cmake)


if(NOT APP_AS_APK)
    add_executable(${PROJECT_NAME} ${SOURCE_FILES})
    add_executable(${GET_OPT_MODEL_BIN} ${GET_OPT_MODEL_SRC_FILES})
    add_executable(${OFFLINE_INFERENCE_3SEC} ${OFFLINE_INFERENCE_SRC_FILES})
    add_executable(${OFFLINE_INFERENCE_2SEC} ${OFFLINE_INFERENCE_SRC_FILES})

    target_compile_definitions(${OFFLINE_INFERENCE_3SEC} PRIVATE INPUT_SAMPLE_COUNT=24000)
    target_compile_definitions(${OFFLINE_INFERENCE_2SEC} PRIVATE INPUT_SAMPLE_COUNT=16000)
else()
    add_compile_definitions(APP_AS_APK)
    add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
    target_link_libraries(${PROJECT_NAME} PRIVATE log)
endif()

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE oboe
    PRIVATE log
    PRIVATE cxxopts::cxxopts
    PRIVATE magic_enum::magic_enum
    PRIVATE anira
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE Boost::pfr
)

target_link_options(${PROJECT_NAME} PRIVATE -Wl,--allow-shlib-undefined)

target_include_directories(
    ${GET_OPT_MODEL_BIN}
    PRIVATE ${ONXX_RT_SRC_DIR}/headers
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_BINARY_DIR}/_deps/onnxruntime-src/headers
)

target_link_libraries(
    ${GET_OPT_MODEL_BIN}
    PRIVATE log
    PRIVATE cxxopts::cxxopts
    PRIVATE magic_enum::magic_enum
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE Boost::pfr
    PRIVATE ${ONXX_RT_SRC_DIR}/jni/arm64-v8a/libonnxruntime.so
    PRIVATE SampleRate::samplerate
    PRIVATE sndfile 
    PRIVATE fftw3
)
target_link_options(${GET_OPT_MODEL_BIN} PRIVATE -Wl,--allow-shlib-undefined)

target_include_directories(
    ${OFFLINE_INFERENCE_3SEC}
    PRIVATE ${ONXX_RT_SRC_DIR}/headers
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_BINARY_DIR}/_deps/onnxruntime-src/headers
)

target_link_libraries(
    ${OFFLINE_INFERENCE_3SEC}
    PRIVATE log
    PRIVATE cxxopts::cxxopts
    PRIVATE magic_enum::magic_enum
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE Boost::pfr
    PRIVATE ${ONXX_RT_SRC_DIR}/jni/arm64-v8a/libonnxruntime.so
    PRIVATE SampleRate::samplerate
    PRIVATE sndfile 
    PRIVATE fftw3
)
target_link_options(${OFFLINE_INFERENCE_3SEC} PRIVATE -Wl,--allow-shlib-undefined)

target_include_directories(
    ${OFFLINE_INFERENCE_2SEC}
    PRIVATE ${ONXX_RT_SRC_DIR}/headers
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_BINARY_DIR}/_deps/onnxruntime-src/headers
)

target_link_libraries(
    ${OFFLINE_INFERENCE_2SEC}
    PRIVATE log
    PRIVATE cxxopts::cxxopts
    PRIVATE magic_enum::magic_enum
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE Boost::pfr
    PRIVATE ${ONXX_RT_SRC_DIR}/jni/arm64-v8a/libonnxruntime.so
    PRIVATE SampleRate::samplerate
    PRIVATE sndfile 
    PRIVATE fftw3
)
target_link_options(${OFFLINE_INFERENCE_2SEC} PRIVATE -Wl,--allow-shlib-undefined)

