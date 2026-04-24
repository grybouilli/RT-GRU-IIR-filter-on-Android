#pragma once

#include <onnxruntime_cxx_api.h>

#include <array>
#include <vector>

struct OrtTensorBuffer {
    OrtTensorBuffer(const Ort::MemoryInfo&         mem_info,
                    const std::array<int64_t, 3>&& tensor_shape) :
        shape{tensor_shape},
        buffer_memory(tensor_shape[0] * tensor_shape[1] * tensor_shape[2], 0.f),
        tensor{Ort::Value::CreateTensor<float>(mem_info,
                                               buffer_memory.data(),
                                               buffer_memory.size(),
                                               shape.data(),
                                               shape.size())} {}

    std::array<int64_t, 3> shape;
    std::vector<float>     buffer_memory;
    Ort::Value             tensor;
};