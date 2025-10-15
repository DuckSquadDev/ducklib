#ifndef TYPES_H
#define TYPES_H

#include <cassert>
#include <wrl/client.h>
#include <dxgi1_4.h>

#include "command_list.h"
#include "../lib/d3dx12.h"

#include "constants.h"
#include "render/render_util.h"

using Microsoft::WRL::ComPtr;

namespace ducklib::render {
constexpr auto MAX_RT_COUNT = 8;

struct Adapter {
    ComPtr<IDXGIAdapter1> dxgi_adapter;
    char name[128] = {};
};

struct Buffer {
    ComPtr<ID3D12Resource1> d3d12_resource;
    uint64_t size;

    void map(void** out_ptr) {
        DL_CHECK_D3D(d3d12_resource->Map(0, nullptr, out_ptr));
    }

    void unmap() {
        d3d12_resource->Unmap(0, nullptr);
    }
};

struct Texture {
    ComPtr<ID3D12Resource1> d3d12_resource;
    uint32_t width;
    uint32_t height;
    Format format;

    void map(void** out_ptr) {
        DL_CHECK_D3D(d3d12_resource.Get()->Map(0, nullptr, out_ptr));
    }

    void unmap() {
        d3d12_resource.Get()->Unmap(0, nullptr);
    }
};

struct Descriptor {
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle;
};

struct DescriptorHeap {
    ComPtr<ID3D12DescriptorHeap> d3d12_heap;
    D3D12_CPU_DESCRIPTOR_HANDLE first_cpu_handle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE first_gpu_handle = {};
    uint16_t descriptor_size;
    DescriptorHeapType type;
    uint32_t count = 0;
    uint32_t allocatedCount = 0;

    auto allocate() -> Descriptor {
        assert(allocatedCount + 1 <= count);

        uint32_t index = allocatedCount;
        ++allocatedCount;

        return Descriptor { cpu_handle(index), gpu_handle(index) };
    }

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
    ComPtr<ID3DBlob> d3d_bytecode_blob;
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
    union {
        BufferDescriptorDesc buffer;
        TextureDescriptorDesc texture;
        StoreImageDescriptorDesc storage_image;
    };
    uint32_t array_size = 1;
    uint32_t array_offset = 0;
    Format format;
    ResourceType type;
};

struct DescriptorSet {};

struct ConstantBindingDesc {
    uint32_t shader_register;
    uint32_t register_space;
    uint32_t num_32bit_values;
};

struct DescriptorBindingDesc {
    uint32_t shader_register = 0;
    uint32_t register_space = 0;
};

// Support for each binding being able to bind multiple descriptor sets/tables is skipped for simplicity
struct DescriptorSetBindingDesc {
    DescriptorSetRangeType type;
    uint32_t base_shader_register = 0;
    uint32_t register_space = 0;
    uint32_t descriptor_count;
    uint32_t descriptor_offset = 0;
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
    uint32_t binding_count = 0;
};

struct BindingSet {
    ComPtr<ID3D12RootSignature> d3d12_signature;
};

struct RasterizerDesc {
    uint32_t msaa_sample_count = 0;
    FillMode fill_mode = FillMode::SOLID;
    CullMode cull_mode = CullMode::BACK;
    FrontFace front_face = FrontFace::CLOCKWISE;
    bool clip_depth = true;
    bool msaa = false;
};

struct RtBlendDesc {
    bool blend_enable = false;
    bool logic_op_enable = false;
    Blend source_blend = Blend::ONE;
    Blend dest_blend = Blend::ZERO;
    BlendOp blend_op = BlendOp::ADD;
    Blend source_blend_alpha = Blend::ONE;
    Blend dest_blend_alpha = Blend::ZERO;
    BlendOp blend_op_alpha = BlendOp::ADD;
    LogicOp logic_op = LogicOp::NOOP;
    uint8_t rt_write_mask = D3D12_COLOR_WRITE_ENABLE_ALL;
};

struct BlendDesc {
    RtBlendDesc rts[MAX_RT_COUNT];
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
    uint32_t aligned_byte_offset = 0;
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
    BlendDesc blend;
    DepthStencilDesc depth_stencil;
    InputLayout input_layout;
    uint32_t rt_count;
    Format rt_formats[MAX_RT_COUNT];
    Format ds_format;
    PrimitiveTopology primitive_topology = PrimitiveTopology::TRIANGLE;
};

struct Pso {
    ComPtr<ID3D12PipelineState> d3d12_pso;
};

struct SwapChain {
    static constexpr auto MAX_BUFFERS = 16;
    
    ComPtr<IDXGISwapChain1> d3d12_swap_chain;
    Descriptor buffer_descriptors[MAX_BUFFERS];
    Texture buffers[MAX_BUFFERS];
    uint64_t frame_index = 0;
    uint8_t buffer_count;

    void present();
    Texture& current_buffer();
    Descriptor current_buffer_descriptor() const;
};

struct Fence {
    ComPtr<ID3D12Fence1> d3d12_fence;
    HANDLE event_handle = {};

    void set_completion_value(uint64_t value);
    void wait();
};

struct CommandList {
    ComPtr<ID3D12CommandAllocator> d3d12_alloc;
    ComPtr<ID3D12GraphicsCommandList2> d3d12_list;
    QueueType type;

    void close();
    void reset(const Pso* pso = nullptr);

    void set_pso(const Pso& pso);
    void set_binding_set(const BindingSet& binding_set);
    void set_descriptor_heaps(uint32_t heap_count, DescriptorHeap** heaps);
    void set_descriptor_set(uint32_t slot, Descriptor base_descriptor);
    void set_viewport(float top_left_x, float top_left_y, float width, float height);
    void set_scissor_rect(int32_t left, int32_t top, int32_t right, int32_t bottom);
    void set_primitive_topology(PrimitiveTopology topology);
    void set_constant_buffer(uint32_t param_index, Buffer constant_buffer);
    void set_vertex_buffer(const Buffer& vertex_buffer, uint32_t stride);
    
    void draw(uint32_t vertex_count, uint32_t offset);

    void copy_texture(Texture& dest, uint32_t dest_x, uint32_t dest_y, uint32_t dest_z, const Texture& source);
    void copy_texture(Texture& dest, uint32_t dest_x, uint32_t dest_y, const Buffer& source);

    void set_rt(const Descriptor& rt_descriptor);
    void clear_rt(const Descriptor& rt_descriptor, const float color[4]);

    void resource_barrier(void* d3d12_resource, ResourceState start_state, ResourceState end_state);
    void resource_barrier(const Texture& texture, ResourceState start_state, ResourceState end_state);
};

struct CommandQueue {
    QueueType type;
    ComPtr<ID3D12CommandQueue> d3d12_queue;

    void signal(const Fence& fence, uint64_t value);
    void execute(const CommandList& list);
};
}

#endif //TYPES_H
