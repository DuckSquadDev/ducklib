#ifndef SHADER_H
#define SHADER_H

#include <cstdlib>
#include <d3dcompiler.h>

#include "types.h"

namespace ducklib::render {
void compile_shader(const wchar_t* filename, ShaderType shader_type, const char* entry_point, Shader* shader_out);
auto to_d3d12_bytecode(const Shader* shader) -> D3D12_SHADER_BYTECODE;
}

#endif //SHADER_H
