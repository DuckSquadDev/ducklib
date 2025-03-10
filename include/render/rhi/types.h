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
    ID3D12Resource1* d3d12_resource = nullptr;
    uint64_t size;
};

struct DescriptorHeap {
    ID3D12DescriptorHeap* d3d12_heap = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE first_cpu_handle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE first_gpu_handle = {};
    uint16_t descriptor_size;
    DescriptorHeapType type;

    auto cpu_handle(uint64_t index) -> D3D12_CPU_DESCRIPTOR_HANDLE {
        auto ptr = reinterpret_cast<uint8_t*>(first_cpu_handle.ptr) + index * descriptor_size;
        return D3D12_CPU_DESCRIPTOR_HANDLE{ reinterpret_cast<SIZE_T>(ptr) };
    }

    auto gpu_handle(uint64_t index) -> D3D12_GPU_DESCRIPTOR_HANDLE {
        auto ptr = reinterpret_cast<uint8_t*>(first_gpu_handle.ptr) + index * descriptor_size;
        return D3D12_GPU_DESCRIPTOR_HANDLE{ reinterpret_cast<SIZE_T>(ptr) };
    }
};

struct Shader {
    ID3DBlob* d3d_bytecode_blob = nullptr;
    ShaderType type;
};

struct BufferDescriptorDesc {
    uint32_t offset = 0;
    uint32_t size = 0;
    uint32_t stride;
    bool raw = false;
};

struct TextureDescriptorDesc {
    uint32_t mip_level_count = 0;
    uint32_t base_mip_level = 0;
    uint32_t plane_slice = 0;
};

struct StoreImageDescriptorDesc {
    uint32_t mip_slice = 0;
    uint32_t plane_slice = 0;
    uint32_t base_w_level = 0;
    uint32_t w_size = 0;
};

struct DescriptorDesc {
    void* resource = nullptr;
    union {
        BufferDescriptorDesc buffer_desc;
        TextureDescriptorDesc texture_desc;
        StoreImageDescriptorDesc storage_image_desc;
    };
    uint32_t array_size = 1;
    uint32_t array_offset = 0;
    Format format;
    ResourceType type;
};

struct Descriptor {
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle;
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
    ShaderVisibility shader_visibility = ShaderVisibility::ALL;
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
    PrimitiveTopology primitive_topology = PrimitiveTopology::TRIANGLE;
};

struct Pso {
    ID3D12PipelineState* d3d12_pso;
};
}

#endif //TYPES_H
