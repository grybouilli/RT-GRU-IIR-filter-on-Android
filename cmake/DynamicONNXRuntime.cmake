include(FetchContent)

# ── Configuration ──────────────────────────────────────────────────────────────
if(ONNX_USE_QNN)
    set(ONXX_RT_URL      "https://repo1.maven.org/maven2/com/microsoft/onnxruntime/onnxruntime-android-qnn/${LIBONNXRUNTIME_VERSION}/onnxruntime-android-qnn-${LIBONNXRUNTIME_VERSION}.aar")
    set(ONXX_RT_SHA256   "397277c3e50326f5972409ca42ab5b4e69172b218557923630d68e55b11d6198") 
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