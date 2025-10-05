#include <filesystem>

#include "render/rhi/shader.h"
#include "render/render_util.h"

namespace ducklib::render {
void compile_shader(const wchar_t* filename, ShaderType shader_type, const char* entry_point, Shader* shader_out) {
    ID3DBlob* code_blob;
    ID3DBlob* errors;
    const char* shader_target = to_d3d_shader_target(shader_type);

    auto d = std::filesystem::current_path();

#ifndef NDEBUG
    auto result = D3DCompileFromFile(
        filename,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entry_point,
        shader_target,
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &code_blob,
        &errors);

    if (FAILED(result)) {
        // TODO: Add formatted HRESULT message to log output
        log(static_cast<const char*>(errors->GetBufferPointer()), LogLevel::ERROR, std::source_location::current());
    }
#else
    if (FAILED(
        D3DCompileFromFile(
            filename,
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entry_point,
            shader_target,
            0,
            0,
            &code_blob,
            &errors))) {
        log(static_cast<const char*>(errors->GetBufferPointer()), LogLevel::ERROR, std::source_location::current());
#endif

    shader_out->d3d_bytecode_blob = code_blob;
    shader_out->type = shader_type;
}

auto to_d3d12_bytecode(const Shader* shader) -> D3D12_SHADER_BYTECODE {
    if (shader && shader->d3d_bytecode_blob) {
        return D3D12_SHADER_BYTECODE{ shader->d3d_bytecode_blob->GetBufferPointer(), shader->d3d_bytecode_blob->GetBufferSize() };
    } else {
        return D3D12_SHADER_BYTECODE{ nullptr, 0 };
    }
}
}
