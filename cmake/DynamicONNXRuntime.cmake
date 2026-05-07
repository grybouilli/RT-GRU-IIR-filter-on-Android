include(FetchContent)

# ── Configuration ──────────────────────────────────────────────────────────────
if(ONNX_USE_QNN)
    set(ONXX_RT_URL      "https://repo1.maven.org/maven2/com/microsoft/onnxruntime/onnxruntime-android-qnn/${LIBONNXRUNTIME_VERSION}/onnxruntime-android-qnn-${LIBONNXRUNTIME_VERSION}.aar")
    set(ONXX_RT_SHA256   "22c9ec9a6d86436fd0e4299dc2ee56e2a89b7285b4d5bde68a888fd6d695f60f") 

    set(QNN_API_URL "https://apigwx-aws.qualcomm.com/qsc/public/v1/api/download/software/sdks/Qualcomm_AI_Runtime_Community/All/2.45.0.260326/v2.45.0.260326.zip")
    set(QNN_API_DL_DIR ${CMAKE_BINARY_DIR}/import)
    set(QNN_API_ZIP_PATH ${QNN_API_DL_DIR}/qnn_api_v2.45.0.zip)
else()
    set(ONXX_RT_URL      "https://repo1.maven.org/maven2/com/microsoft/onnxruntime/onnxruntime-android/${LIBONNXRUNTIME_VERSION}/onnxruntime-android-${LIBONNXRUNTIME_VERSION}.aar")
    set(ONXX_RT_SHA256   "bc461499a735653dff285a6a3477d28b9cfd119a09c7753eaf003426b577f223") 
endif()
set(ONXX_RT_NAME     "onnxruntime")     # Name used internally by FetchContent

# ── Download & extract ─────────────────────────────────────────────────────────
FetchContent_Declare(
    ${ONXX_RT_NAME}
    URL        ${ONXX_RT_URL}
    DOWNLOAD_NAME   "onnxruntime-android.zip"
    URL_HASH   SHA256=${ONXX_RT_SHA256}
    DOWNLOAD_NO_EXTRACT FALSE            # Ensure the archive is extracted
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(${ONXX_RT_NAME})

# ── Locate the .a file inside the extracted folder ─────────────────────────────
# FetchContent extracts into: <build_dir>/_deps/mylib-src/
# Adjust the sub-path to match the actual layout of your archive.
set(ONXX_RT_SRC_DIR  "${${ONXX_RT_NAME}_SOURCE_DIR}")   # e.g. _deps/mylib-src

# ── Create an IMPORTED target so the rest of the build uses it cleanly ─────────
if(NOT TARGET Ort::Ort)
    add_library(Ort::Ort STATIC IMPORTED GLOBAL)
    if(CMAKE_ANDROID_ARCH_ABI MATCHES "armeabi-v7a")
        set_target_properties(Ort::Ort PROPERTIES
            IMPORTED_LOCATION             "${ONXX_RT_SRC_DIR}/jni/armeabi-v7a/libonnxruntime.so"
            INTERFACE_INCLUDE_DIRECTORIES "${ONXX_RT_SRC_DIR}/headers"
        )
        elseif(CMAKE_ANDROID_ARCH_ABI MATCHES "arm64-v8a")
        set_target_properties(Ort::Ort PROPERTIES
            IMPORTED_LOCATION             "${ONXX_RT_SRC_DIR}/jni/arm64-v8a/libonnxruntime.so"
            INTERFACE_INCLUDE_DIRECTORIES "${ONXX_RT_SRC_DIR}/headers"
        )
    else()
        message( FATAL_ERROR "Unknown Android architecture : ${CMAKE_ANDROID_ARCH_ABI} ( supported values: armeabi-v7a , arm64-v8a )")
    endif()
endif()

message(STATUS "Ort static library : ${ONXX_RT_SRC_DIR}")
message(STATUS "Ort include dir    : ${ONXX_RT_SRC_DIR}/headers")

# Download QNN API if needed

if(QNN_API_URL)
    file(DOWNLOAD ${QNN_API_URL} ${QNN_API_ZIP_PATH} STATUS QNN_API_DOWNLOAD_STATUS SHOW_PROGRESS)
    execute_process(
            COMMAND unzip ${QNN_API_ZIP_PATH} 
            WORKING_DIRECTORY ${QNN_API_DL_DIR}/)
endif()