#include "render/rhi/shader.h"

namespace ducklib::render {
void compile_shader(const wchar_t* filename, ShaderType shader_type, const char* entry_point, Shader* shader_out) {
    ID3DBlob* code_blob;
    ID3DBlob* errors;
    const char* shader_target = to_d3d_shader_target(shader_type);

    if (FAILED(
        D3DCompileFromFile(filename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry_point, shader_target, 0, 0, &code_blob, &errors))) {
        std::abort();
    }

    shader_out->d3d_bytecode_blob = code_blob;
    shader_out->type = shader_type;
}

auto to_d3d12_bytecode(const Shader* shader) -> D3D12_SHADER_BYTECODE {
    if (shader->d3d_bytecode_blob) {
        return D3D12_SHADER_BYTECODE{ shader->d3d_bytecode_blob->GetBufferPointer(), shader->d3d_bytecode_blob->GetBufferSize() };
    } else {
        return D3D12_SHADER_BYTECODE{ nullptr, 0 };
    }
}
}
