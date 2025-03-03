#ifndef TYPES_H
#define TYPES_H

#include <wrl/client.h>
#include <dxgi1_4.h>
#include "../lib/d3dx12.h"

#include "constants.h"

using Microsoft::WRL::ComPtr;

namespace ducklib::render {
struct Adapter {
    ComPtr<IDXGIAdapter1> dxgi_adapter = nullptr;
    char name[128] = {};
};

struct Buffer {
    ID3D12Resource1* d3d12_resource;
};

struct DescriptorHeap {
    ID3D12DescriptorHeap* d3d12_heap;
};

struct Shader {
    ID3DBlob* d3d_bytecode_blob;
    ShaderType type;
};

struct BufferDescriptor {
    uint32_t offset;
    uint32_t size;
    uint32_t stride;
    bool raw;
};

struct TextureDescriptor {
    uint32_t mip_level_count;
    uint32_t base_mip_level;
    uint32_t plane_slice;
};

struct StoreImageDescriptor {
    uint32_t mip_slice;
    uint32_t plane_slice;
    uint32_t base_w_level;
    uint32_t w_size;
};

struct Descriptor {
    void* resource;
    union {
        BufferDescriptor buffer_desc;
        TextureDescriptor texture_desc;
        StoreImageDescriptor storage_image_desc;
    };
    uint32_t array_size;
    Format format;
    DescriptorType type;
};

struct DescriptorSet {};

struct ConstantBindingDesc {
    uint32_t shader_register;
    uint32_t register_space;
    uint32_t num_32bit_values;
};

struct DescriptorBindingDesc {
    uint32_t shader_register;
    uint32_t register_space;
};

// Support for each binding being able to bind multiple descriptor sets/tables is skipped for simplicity
struct DescriptorSetBindingDesc {
    uint32_t base_shader_register;
    uint32_t register_space;
    uint32_t descriptor_count;
    uint32_t descriptor_offset;
};

struct BindingDesc {
    union {
        ConstantBindingDesc constant_binding;
        DescriptorBindingDesc descriptor_binding;
        DescriptorSetBindingDesc descriptor_set_binding;
    };
    BindingType type;
    ShaderVisibility shader_visibility;
};

struct BindingSetDesc {
    BindingDesc bindings[64];
    uint32_t binding_count;
};

struct BindingSet {
    ID3D12RootSignature* d3d12_signature;
};

struct RasterizerDesc {
    uint32_t msaa_sample_count = 0;
    FillMode fill_mode = FillMode::SOLID;
    CullMode cull_mode = CullMode::BACK;
    FrontFace front_face = FrontFace::CLOCKWISE;
    bool clip_depth = true;
    bool msaa = false;
};

struct DepthStencilDesc {
    DepthComparison depth_comparison = DepthComparison::LTEQ;
    uint8_t stencil_read_mask = 0xff;
    uint8_t stencil_write_mask = 0xff;
    bool enable_depth = true;
    bool write_depth = true;
    bool enable_stencil = false;
};

struct InputLayoutItem {
    const char* semantic_name;
    uint32_t semantic_index = 0;
    uint32_t input_slot = 0;
    uint32_t aligned_byte_offset = D3D12_APPEND_ALIGNED_ELEMENT;
    uint32_t instances_per_step = 0;
    Format format;
    InputSlotType slot_type = InputSlotType::PER_VERTEX_DATA;
};

struct InputLayout {
    InputLayoutItem elements[32];
    uint32_t element_count = 0;
};

struct PsoDesc {
    Shader* vertex_shader = nullptr;
    Shader* pixel_shader = nullptr;
    Shader* geometry_shader = nullptr;
    Shader* hull_shader = nullptr;
    Shader* domain_shader = nullptr;
    RasterizerDesc rasterizer;
    DepthStencilDesc depth_stencil;
    InputLayout input_layout;
    uint32_t rt_count;
    Format rt_formats[8];
    Format ds_format;
    PrimitiveTopology primitive_topology;
};

struct Pso {
    ID3D12PipelineState* d3d12_pso;
};
}

#endif //TYPES_H
