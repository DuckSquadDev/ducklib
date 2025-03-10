#ifndef FORMAT_H
#define FORMAT_H
#include <cstdlib>
#include <cstdint>

namespace ducklib::render {
enum class QueueType {
    GRAPHICS,
    TRANSFER,
    COMPUTE
};

enum class HeapType {
    DEFAULT,
    UPLOAD
};

enum class DescriptorHeapType {
    CBV_SRV_UAV,
    SAMPLER,
    RT,
    DS
};

#undef DOMAIN
enum class BindingStage {
    VERTEX = 1,
    PIXEL = 2,
    GEOMETRY = 4,
    HULL = 8,
    DOMAIN = 16,
    COMPUTE = 32,
    ALL = 255
};

enum class ShaderType {
    VERTEX,
    PIXEL,
    GEOMETRY,
    HULL,
    DOMAIN,
    COMPUTE
};

enum class ShaderVisibility {
    ALL,
    VERTEX,
    HULl,
    DOMAIN,
    GEOMETRY,
    PIXEL
};

enum class BindingType {
    CONSTANT,
    BUFFER_DESCRIPTOR, // "BUFFER_DESCRIPTOR"?
    SRV_DESCRIPTOR, // TODO: Rename? "READ_DESCRIPTOR"?
    UAV_DESCRIPTOR, // "WRITE_DESCRIPTOR"?
    DESCRIPTOR_SET
};

enum class ResourceType {
    BUFFER,
    TEXTURE_1D,
    TEXTURE_2D,
    TEXTURE_3D,
    TEXTURE_CUBE
};

enum class FillMode {
    SOLID,
    WIREFRAME
};

enum class CullMode {
    BACK,
    FRONT,
    NONE
};

enum class FrontFace {
    CLOCKWISE,
    COUNTER_CLOCKWISE
};

enum class DepthComparison {
    LTEQ,
    LT,
    GTEQ,
    GT,
    EQ,
    NEQ,
    ALWAYS,
    NEVER
};

enum class InputSlotType {
    PER_VERTEX_DATA,
    PER_INSTANCE_DATA
};

enum class PrimitiveTopology {
    UNDEFINED,
    POINT,
    LINE,
    TRIANGLE,
    PATCH
};

enum class Format : uint32_t {
    UNKNOWN = 0,
    R32G32B32A32_TYPELESS,
    R32G32B32A32_FLOAT,
    R32G32B32A32_UINT,
    R32G32B32A32_SINT,
    R32G32B32_TYPELESS,
    R32G32B32_FLOAT,
    R32G32B32_UINT,
    R32G32B32_SINT,
    R16G16B16A16_TYPELESS,
    R16G16B16A16_FLOAT,
    R16G16B16A16_UINT,
    R16G16B16A16_SINT,
    R16G16B16A16_UNORM,
    R16G16B16A16_SNORM,
    R32G32_TYPELESS,
    R32G32_FLOAT,
    R32G32_UINT,
    R32G32_SINT,
    R32G8X24_TYPELESS,
    D32_FLOAT_S8X24_UINT,
    R32_FLOAT_X8X24_TYPELESS,
    X32_TYPELESS_G8X24_UINT,
    R10G10B10A2_TYPELESS,
    R10G10B10A2_UNORM,
    R10G10B10A2_UINT,
    R11G11B10_FLOAT,
    R8G8B8A8_TYPELESS,
    R8G8B8A8_UNORM,
    R8G8B8A8_UNORM_SRGB,
    R8G8B8A8_UINT,
    R8G8B8A8_SNORM,
    R8G8B8A8_SINT,
    R16G16_TYPELESS,
    R16G16_FLOAT,
    R16G16_UNORM,
    R16G16_UINT,
    R16G16_SNORM,
    R16G16_SINT,
    R32_TYPELESS,
    D32_FLOAT,
    R32_FLOAT,
    R32_UINT,
    R32_SINT,
    R24G8_TYPELESS,
    D24_UNORM_S8_UINT,
    R24_UNORM_X8_TYPELESS,
    X24_TYPELESS_G8_UINT,
    B8G8R8A8_TYPELESS,
    B8G8R8A8_UNORM,
    B8G8R8A8_UNORM_SRGB,
    B8G8R8X8_TYPELESS,
    B8G8R8X8_UNORM,
    B8G8R8X8_UNORM_SRGB,
    R8G8_TYPELESS,
    R8G8_UNORM,
    R8G8_UINT,
    R8G8_SNORM,
    R8G8_SINT,
    R16_TYPELESS,
    R16_FLOAT,
    D16_UNORM,
    R16_UNORM,
    R16_UINT,
    R16_SNORM,
    R16_SINT,
    R8_TYPELESS,
    R8_UNORM,
    R8_UINT,
    R8_SNORM,
    R8_SINT,
    A8_UNORM,
    //R1_UNORM					// ignore? weird undocumented stuff
    R9G9B9E5_SHAREDEXP,
    R8G8_B8G8_UNORM,
    G8R8_G8B8_UNORM,
    BC1_TYPELESS, // these formats are block compression
    BC1_UNORM,
    BC1_UNORM_SRGB,
    BC2_TYPELESS,
    BC2_UNORM,
    BC2_UNORM_SRGB,
    BC3_TYPELESS,
    BC3_UNORM,
    BC3_UNORM_SRGB,
    BC4_TYPELESS,
    BC4_UNORM,
    BC4_SNORM,
    BC5_TYPELESS,
    BC5_UNORM,
    BC5_SNORM,
    BC6H_TYPELESS,
    BC6H_UF16,
    BC6H_SF16,
    BC7_TYPELESS,
    BC7_UNORM,
    BC7_UNORM_SRGB,
    B5G6R5_UNORM, // only for D3D 11.1+ (Win8+)
    B5G5R5A1_UNORM, // only for D3D 11.1+ (Win8+)
    R10G10B10_XR_BIAS_A2_UNORM,
    ENTRY_COUNT // what else can you do? :'(
};

inline auto to_d3d12_stage(BindingStage stage) -> D3D12_SHADER_VISIBILITY {
    switch (stage) {
    case BindingStage::VERTEX: return D3D12_SHADER_VISIBILITY_VERTEX;
    case BindingStage::PIXEL: return D3D12_SHADER_VISIBILITY_PIXEL;
    case BindingStage::GEOMETRY: return D3D12_SHADER_VISIBILITY_GEOMETRY;
    case BindingStage::HULL: return D3D12_SHADER_VISIBILITY_HULL;
    case BindingStage::DOMAIN: return D3D12_SHADER_VISIBILITY_DOMAIN;
    case BindingStage::ALL: return D3D12_SHADER_VISIBILITY_ALL;
    default: std::abort();
    }
}

inline auto to_d3d12_queue_type(QueueType type) {
    switch (type) {
    case QueueType::GRAPHICS: return D3D12_COMMAND_LIST_TYPE_DIRECT;
    case QueueType::COMPUTE: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    case QueueType::TRANSFER: return D3D12_COMMAND_LIST_TYPE_COPY;
    default: std::abort();
    }
}

inline auto to_d3d12_descriptor_heap_type(DescriptorHeapType heap_type) -> D3D12_DESCRIPTOR_HEAP_TYPE {
    switch (heap_type) {
    case DescriptorHeapType::CBV_SRV_UAV: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    case DescriptorHeapType::SAMPLER: return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    case DescriptorHeapType::RT: return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    case DescriptorHeapType::DS: return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    default: std::abort();
    }
}

inline auto to_d3d12_binding_type(BindingType type) -> D3D12_ROOT_PARAMETER_TYPE {
    switch (type) {
    case BindingType::CONSTANT: return D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    case BindingType::BUFFER_DESCRIPTOR: return D3D12_ROOT_PARAMETER_TYPE_CBV;
    case BindingType::SRV_DESCRIPTOR: return D3D12_ROOT_PARAMETER_TYPE_SRV;
    case BindingType::UAV_DESCRIPTOR: return D3D12_ROOT_PARAMETER_TYPE_UAV;
    case BindingType::DESCRIPTOR_SET: return D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    default: std::abort();
    }
}

inline auto to_d3d12_view_dimension(ResourceType type, uint32_t array_size) -> D3D12_SRV_DIMENSION {
    switch (type) {
    case ResourceType::BUFFER: return D3D12_SRV_DIMENSION_BUFFER;
    case ResourceType::TEXTURE_1D:
        if (array_size > 1) {
            return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
        } else {
            return D3D12_SRV_DIMENSION_TEXTURE1D;
        }
    case ResourceType::TEXTURE_2D:
        if (array_size > 1) {
            return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        } else {
            return D3D12_SRV_DIMENSION_TEXTURE2D;
        }
    case ResourceType::TEXTURE_3D: return D3D12_SRV_DIMENSION_TEXTURE3D;
    case ResourceType::TEXTURE_CUBE:
        if (array_size > 1) {
            return D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
        } else {
            return D3D12_SRV_DIMENSION_TEXTURECUBE;
        }
    default: std::abort();
    }
}

inline auto to_d3d_shader_target(ShaderType type) -> const char* {
    switch (type) {
    case ShaderType::VERTEX: return "vs_5_0";
    case ShaderType::PIXEL: return "ps_5_0";
    case ShaderType::GEOMETRY: return "gs_5_0";
    case ShaderType::DOMAIN: return "ds_5_0";
    case ShaderType::HULL: return "hs_5_0";
    case ShaderType::COMPUTE: return "cs_5_0";
    default: std::abort();
    }
}

inline auto to_d3d12_fill_mode(FillMode fill_mode) -> D3D12_FILL_MODE {
    switch (fill_mode) {
    case FillMode::WIREFRAME: return D3D12_FILL_MODE_WIREFRAME;
    case FillMode::SOLID: return D3D12_FILL_MODE_SOLID;
    default: std::abort();
    }
}

inline auto to_d3d12_cull_mode(CullMode cull_mode) -> D3D12_CULL_MODE {
    switch (cull_mode) {
    case CullMode::NONE: return D3D12_CULL_MODE_NONE;
    case CullMode::FRONT: return D3D12_CULL_MODE_FRONT;
    case CullMode::BACK: return D3D12_CULL_MODE_BACK;
    default: std::abort();
    }
}

inline auto to_d3d12_front_face(FrontFace front_face) -> BOOL {
    switch (front_face) {
    case FrontFace::CLOCKWISE: return FALSE;
    case FrontFace::COUNTER_CLOCKWISE: return TRUE;
    default: std::abort();
    }
}

inline auto to_d3d12_depth_comparison(DepthComparison depth_comparison) -> D3D12_COMPARISON_FUNC {
    switch (depth_comparison) {
    case DepthComparison::LTEQ: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
    case DepthComparison::LT: return D3D12_COMPARISON_FUNC_LESS;
    case DepthComparison::GTEQ: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    case DepthComparison::GT: return D3D12_COMPARISON_FUNC_GREATER;
    case DepthComparison::EQ: return D3D12_COMPARISON_FUNC_EQUAL;
    case DepthComparison::NEQ: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
    case DepthComparison::ALWAYS: return D3D12_COMPARISON_FUNC_ALWAYS;
    case DepthComparison::NEVER: return D3D12_COMPARISON_FUNC_NEVER;
    default: std::abort();
    }
}

inline auto to_d3d12_input_slot_type(InputSlotType slot_type) -> D3D12_INPUT_CLASSIFICATION {
    switch (slot_type) {
    case InputSlotType::PER_VERTEX_DATA: return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    case InputSlotType::PER_INSTANCE_DATA: return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    default: std::abort();
    }
}

inline auto to_d3d12_primitive_topology(PrimitiveTopology topology) -> D3D12_PRIMITIVE_TOPOLOGY_TYPE {
    switch (topology) {
    case PrimitiveTopology::UNDEFINED: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    case PrimitiveTopology::POINT: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    case PrimitiveTopology::LINE: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    case PrimitiveTopology::TRIANGLE: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case PrimitiveTopology::PATCH: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    default: std::abort();
    }
}

static DXGI_FORMAT dxgi_format_map[] = {
    DXGI_FORMAT_UNKNOWN,
    DXGI_FORMAT_R32G32B32A32_TYPELESS,
    DXGI_FORMAT_R32G32B32A32_FLOAT,
    DXGI_FORMAT_R32G32B32A32_UINT,
    DXGI_FORMAT_R32G32B32A32_SINT,
    DXGI_FORMAT_R32G32B32_TYPELESS,
    DXGI_FORMAT_R32G32B32_FLOAT,
    DXGI_FORMAT_R32G32B32_UINT,
    DXGI_FORMAT_R32G32B32_SINT,
    DXGI_FORMAT_R16G16B16A16_TYPELESS,
    DXGI_FORMAT_R16G16B16A16_FLOAT,
    DXGI_FORMAT_R16G16B16A16_UINT,
    DXGI_FORMAT_R16G16B16A16_SINT,
    DXGI_FORMAT_R16G16B16A16_UNORM,
    DXGI_FORMAT_R16G16B16A16_SNORM,
    DXGI_FORMAT_R32G32_TYPELESS,
    DXGI_FORMAT_R32G32_FLOAT,
    DXGI_FORMAT_R32G32_UINT,
    DXGI_FORMAT_R32G32_SINT,
    DXGI_FORMAT_R32G8X24_TYPELESS,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
    DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
    DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,
    DXGI_FORMAT_R10G10B10A2_TYPELESS,
    DXGI_FORMAT_R10G10B10A2_UNORM,
    DXGI_FORMAT_R10G10B10A2_UINT,
    DXGI_FORMAT_R11G11B10_FLOAT,
    DXGI_FORMAT_R8G8B8A8_TYPELESS,
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    DXGI_FORMAT_R8G8B8A8_UINT,
    DXGI_FORMAT_R8G8B8A8_SNORM,
    DXGI_FORMAT_R8G8B8A8_SINT,
    DXGI_FORMAT_R16G16_TYPELESS,
    DXGI_FORMAT_R16G16_FLOAT,
    DXGI_FORMAT_R16G16_UNORM,
    DXGI_FORMAT_R16G16_UINT,
    DXGI_FORMAT_R16G16_SNORM,
    DXGI_FORMAT_R16G16_SINT,
    DXGI_FORMAT_R32_TYPELESS,
    DXGI_FORMAT_D32_FLOAT,
    DXGI_FORMAT_R32_FLOAT,
    DXGI_FORMAT_R32_UINT,
    DXGI_FORMAT_R32_SINT,
    DXGI_FORMAT_R24G8_TYPELESS,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
    DXGI_FORMAT_X24_TYPELESS_G8_UINT,
    DXGI_FORMAT_B8G8R8A8_TYPELESS,
    DXGI_FORMAT_B8G8R8A8_UNORM,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
    DXGI_FORMAT_B8G8R8X8_TYPELESS,
    DXGI_FORMAT_B8G8R8X8_UNORM,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
    DXGI_FORMAT_R8G8_TYPELESS,
    DXGI_FORMAT_R8G8_UNORM,
    DXGI_FORMAT_R8G8_UINT,
    DXGI_FORMAT_R8G8_SNORM,
    DXGI_FORMAT_R8G8_SINT,
    DXGI_FORMAT_R16_TYPELESS,
    DXGI_FORMAT_R16_FLOAT,
    DXGI_FORMAT_D16_UNORM,
    DXGI_FORMAT_R16_UNORM,
    DXGI_FORMAT_R16_UINT,
    DXGI_FORMAT_R16_SNORM,
    DXGI_FORMAT_R16_SINT,
    DXGI_FORMAT_R8_TYPELESS,
    DXGI_FORMAT_R8_UNORM,
    DXGI_FORMAT_R8_UINT,
    DXGI_FORMAT_R8_SNORM,
    DXGI_FORMAT_R8_SINT,
    DXGI_FORMAT_A8_UNORM,
    // DXGI_FORMAT_R1_UNORM,
    DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
    DXGI_FORMAT_R8G8_B8G8_UNORM,
    DXGI_FORMAT_G8R8_G8B8_UNORM,
    DXGI_FORMAT_BC1_TYPELESS,
    DXGI_FORMAT_BC1_UNORM,
    DXGI_FORMAT_BC1_UNORM_SRGB,
    DXGI_FORMAT_BC2_TYPELESS,
    DXGI_FORMAT_BC2_UNORM,
    DXGI_FORMAT_BC2_UNORM_SRGB,
    DXGI_FORMAT_BC3_TYPELESS,
    DXGI_FORMAT_BC3_UNORM,
    DXGI_FORMAT_BC3_UNORM_SRGB,
    DXGI_FORMAT_BC4_TYPELESS,
    DXGI_FORMAT_BC4_UNORM,
    DXGI_FORMAT_BC4_SNORM,
    DXGI_FORMAT_BC5_TYPELESS,
    DXGI_FORMAT_BC5_UNORM,
    DXGI_FORMAT_BC5_SNORM,
    DXGI_FORMAT_BC6H_TYPELESS,
    DXGI_FORMAT_BC6H_UF16,
    DXGI_FORMAT_BC6H_SF16,
    DXGI_FORMAT_BC7_TYPELESS,
    DXGI_FORMAT_BC7_UNORM,
    DXGI_FORMAT_BC7_UNORM_SRGB,
    DXGI_FORMAT_B5G6R5_UNORM,
    DXGI_FORMAT_B5G5R5A1_UNORM,
    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM
};

inline DXGI_FORMAT to_d3d12_format(Format format) {
    return dxgi_format_map[static_cast<uint32_t>(format)];
}
}

#endif //FORMAT_H
