#ifndef D3D12_UTILITY_H
#define D3D12_UTILITY_H
#include <d3d12.h>
#include <stdexcept>

#include "render/core/descriptor_set.h"

namespace ducklib::render {
inline auto to_d3d12_stage(BindingStage stage) -> D3D12_SHADER_VISIBILITY {
    switch (stage) {
    case BindingStage::VERTEX: return D3D12_SHADER_VISIBILITY_VERTEX;
    case BindingStage::PIXEL: return D3D12_SHADER_VISIBILITY_PIXEL;
    case BindingStage::GEOMETRY: return D3D12_SHADER_VISIBILITY_GEOMETRY;
    case BindingStage::HULL: return D3D12_SHADER_VISIBILITY_HULL;
    case BindingStage::DOMAIN: return D3D12_SHADER_VISIBILITY_DOMAIN;
    case BindingStage::ALL: return D3D12_SHADER_VISIBILITY_ALL;
    default: throw std::invalid_argument("Invalid BindingStage");
    }
}

inline auto to_d3d12_srv_dimension(DescriptorType type) -> D3D12_SRV_DIMENSION {
    switch (type) {
    case DescriptorType::BUFFER: return D3D12_SRV_DIMENSION_BUFFER;
    default: return D3D12_SRV_DIMENSION_UNKNOWN;
    }
}
}

#endif //D3D12_UTILITY_H
