#include <wrl/client.h>
#include <stdexcept>

#include "core/win/win_app_window.h"
#include "render/rhi/device.h"
#include "render/rhi/command_list.h"
#include "render/rhi/shader.h"

namespace ducklib::render {
void Device::create_queue(QueueType type, CommandQueue* out_queue) {
    D3D12_COMMAND_QUEUE_DESC queue_desc = {};
    queue_desc.Type = to_d3d12_queue_type(type);
    queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.NodeMask = 0;

    if (FAILED(d3d12_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&out_queue->d3d12_queue)))) {
        std::abort();
    }
}

void Device::create_command_list(QueueType queue_type, CommandList* out_list) {
    out_list->type = queue_type;
    auto d3d12_type = to_d3d12_queue_type(queue_type);

    if (FAILED(d3d12_device->CreateCommandAllocator(d3d12_type, IID_PPV_ARGS(&out_list->d3d12_alloc)))) {
        throw std::runtime_error("failed to create D3D12 command allocator");
    }

    if (FAILED(d3d12_device->CreateCommandList(0, d3d12_type, out_list->d3d12_alloc, nullptr, IID_PPV_ARGS(&out_list->d3d12_list)))) {
        throw std::runtime_error("failed to create D3D12 command list");
    }
}

void Device::create_binding_set(BindingSetDesc binding_set_desc) {
    D3D12_ROOT_PARAMETER d3d12_params[64] = {};
    D3D12_DESCRIPTOR_RANGE d3d12_descriptor_ranges[64] = {};
    uint32_t i_ranges = 0;

    for (uint32_t i = 0; i < binding_set_desc.binding_count; ++i) {
        
        switch (binding_set_desc.bindings[i].type) {
        case BindingType::CONSTANT: {
            d3d12_params[i].ParameterType = to_d3d12_binding_type(binding_set_desc.bindings[i].type);
            d3d12_params[i].Constants.Num32BitValues = binding_set_desc.bindings[i].constant_binding.num_32bit_values;
            d3d12_params[i].Constants.RegisterSpace = binding_set_desc.bindings[i].constant_binding.register_space;
            d3d12_params[i].Constants.ShaderRegister = binding_set_desc.bindings[i].constant_binding.shader_register;
            break;
        }
        case BindingType::BUFFER_DESCRIPTOR:
        case BindingType::SRV_DESCRIPTOR:
        case BindingType::UAV_DESCRIPTOR: {
            d3d12_params[i].ParameterType = to_d3d12_binding_type(binding_set_desc.bindings[i].type);
            d3d12_params[i].Descriptor.RegisterSpace = binding_set_desc.bindings[i].descriptor_binding.register_space;
            d3d12_params[i].Descriptor.ShaderRegister = binding_set_desc.bindings[i].descriptor_binding.shader_register;
            break;
        }
        case BindingType::DESCRIPTOR_SET: {
            DescriptorSetBindingDesc& set_binding = binding_set_desc.bindings[i].descriptor_set_binding;

            d3d12_params[i].ParameterType = to_d3d12_binding_type(binding_set_desc.bindings[i].type);
            d3d12_params[i].DescriptorTable.NumDescriptorRanges = 1;
            d3d12_params[i].DescriptorTable.pDescriptorRanges = &d3d12_descriptor_ranges[i_ranges];
            d3d12_descriptor_ranges[i_ranges].RegisterSpace = set_binding.register_space;
            d3d12_descriptor_ranges[i_ranges].BaseShaderRegister = set_binding.base_shader_register;
            d3d12_descriptor_ranges[i_ranges].NumDescriptors = set_binding.descriptor_count;
            d3d12_descriptor_ranges[i_ranges].OffsetInDescriptorsFromTableStart = set_binding.descriptor_offset;
            ++i_ranges;
            break;
        }
        default:
            std::abort();
        }
    }

    ID3DBlob* signature_blob = {};
    ID3DBlob* errors = {};
    D3D12_ROOT_SIGNATURE_DESC signature_desc = {};

    signature_desc.NumParameters = binding_set_desc.binding_count;
    signature_desc.pParameters = d3d12_params;
    signature_desc.NumStaticSamplers = 0;
    signature_desc.pStaticSamplers = nullptr;
    signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
    
    if (FAILED(D3D12SerializeRootSignature(&signature_desc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature_blob, &errors))) {
        std::abort();
    }

    ID3D12RootSignature* d3d12_signature = {};
    
    if (FAILED(d3d12_device->CreateRootSignature(
        0,
        signature_blob->GetBufferPointer(),
        signature_blob->GetBufferSize(),
        IID_PPV_ARGS(&d3d12_signature)))) {
        std::abort();
    }
}

void Device::create_pso(BindingSet* binding_set, PsoDesc* pso_desc, Pso* pso_out) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12_pso_desc = {};
    D3D12_INPUT_ELEMENT_DESC d3d12_input_elements[32] = {};

    d3d12_pso_desc.pRootSignature = binding_set->d3d12_signature;
    d3d12_pso_desc.VS = to_d3d12_bytecode(pso_desc->vertex_shader);
    d3d12_pso_desc.PS = to_d3d12_bytecode(pso_desc->pixel_shader);
    d3d12_pso_desc.GS = to_d3d12_bytecode(pso_desc->geometry_shader);
    d3d12_pso_desc.DS = to_d3d12_bytecode(pso_desc->domain_shader);
    d3d12_pso_desc.HS = to_d3d12_bytecode(pso_desc->hull_shader);
    d3d12_pso_desc.RasterizerState.FillMode = to_d3d12_fill_mode(pso_desc->rasterizer.fill_mode);
    d3d12_pso_desc.RasterizerState.CullMode = to_d3d12_cull_mode(pso_desc->rasterizer.cull_mode);
    d3d12_pso_desc.RasterizerState.FrontCounterClockwise = to_d3d12_front_face(pso_desc->rasterizer.front_face);
    d3d12_pso_desc.RasterizerState.DepthBias = 0;
    d3d12_pso_desc.RasterizerState.DepthBiasClamp = 0.0f;
    d3d12_pso_desc.RasterizerState.SlopeScaledDepthBias = 0.0f;
    d3d12_pso_desc.RasterizerState.DepthClipEnable = pso_desc->rasterizer.clip_depth;
    d3d12_pso_desc.RasterizerState.MultisampleEnable = pso_desc->rasterizer.msaa;
    d3d12_pso_desc.RasterizerState.AntialiasedLineEnable = false;
    d3d12_pso_desc.RasterizerState.ForcedSampleCount = 0;
    d3d12_pso_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    for (uint32_t i = 0; i < pso_desc->input_layout.element_count; ++i) {
        InputLayoutItem& input_element = pso_desc->input_layout.elements[i];
        d3d12_input_elements[i].SemanticName = input_element.semantic_name;
        d3d12_input_elements[i].SemanticIndex = input_element.semantic_index;
        d3d12_input_elements[i].InputSlot = input_element.input_slot;
        d3d12_input_elements[i].InstanceDataStepRate = input_element.instances_per_step;
        d3d12_input_elements[i].AlignedByteOffset = input_element.aligned_byte_offset;
        d3d12_input_elements[i].Format = to_d3d12_format(input_element.format);
        d3d12_input_elements[i].InputSlotClass = to_d3d12_input_slot_type(input_element.slot_type);
    }
    
    d3d12_pso_desc.InputLayout.NumElements = pso_desc->input_layout.element_count;
    d3d12_pso_desc.InputLayout.pInputElementDescs = d3d12_input_elements;

    d3d12_pso_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    d3d12_pso_desc.PrimitiveTopologyType = to_d3d12_primitive_topology(pso_desc->primitive_topology);
    d3d12_pso_desc.NumRenderTargets = pso_desc->rt_count;

    for (uint32_t i = 0; i < pso_desc->rt_count; ++i) {
        d3d12_pso_desc.RTVFormats[i] = to_d3d12_format(pso_desc->rt_formats[i]);
    }

    d3d12_pso_desc.DSVFormat = to_d3d12_format(pso_desc->ds_format);
    d3d12_pso_desc.SampleDesc.Count = 1;
    d3d12_pso_desc.SampleDesc.Quality = 0;
    d3d12_pso_desc.NodeMask = 0;
    d3d12_pso_desc.CachedPSO.pCachedBlob = nullptr;
    d3d12_pso_desc.CachedPSO.CachedBlobSizeInBytes = 0;
    d3d12_pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
#ifdef _DEBUG
    d3d12_pso_desc.Flags |= D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
#endif
    
    if (FAILED(d3d12_device->CreateGraphicsPipelineState(&d3d12_pso_desc, IID_PPV_ARGS(&pso_out->d3d12_pso)))) {
        std::abort();
    }
}

void Device::create_descriptor_heap(DescriptorType type, uint32_t count, DescriptorHeap* out_heap) {
    D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};

    heap_desc.Type = to_d3d12_descriptor_type(type);
    heap_desc.NumDescriptors = count;
    heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    
    if (FAILED(d3d12_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&out_heap->d3d12_heap)))) {
        std::abort();
    }
}

void Device::create_buffer(uint64_t byte_size, Buffer* out_buffer, HeapType heap_type) {
    D3D12_HEAP_PROPERTIES heap_props = {};
    D3D12_RESOURCE_DESC resource_desc = {};
    D3D12_RESOURCE_STATES init_states = {};

    if (heap_type == HeapType::DEFAULT) {
        heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
        init_states = D3D12_RESOURCE_STATE_COMMON;
    } else {
        heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;
        init_states = D3D12_RESOURCE_STATE_GENERIC_READ;
    }

    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resource_desc.Width = byte_size;
    resource_desc.Format = DXGI_FORMAT_UNKNOWN;
    resource_desc.Height = 1;
    resource_desc.DepthOrArraySize = 1;
    resource_desc.MipLevels = 1;
    resource_desc.SampleDesc.Count = 1;
    resource_desc.SampleDesc.Quality = 0;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    if (FAILED(d3d12_device->CreateCommittedResource(
        &heap_props,
        D3D12_HEAP_FLAG_NONE,
        &resource_desc,
        init_states,
        nullptr,
        IID_PPV_ARGS(&out_buffer->d3d12_resource)))) {
        std::abort();
    }
}

// auto Device::create_descriptor_set_layout(
//     std::span<DescriptorSetLayoutItem> layout_items)
//     -> DescriptorSetLayout* {
//     auto layout = new D3d12DescriptorSetLayout{ layout_items };
//     descriptor_set_layouts.push_back(layout);
//
//     return layout;
// }

// DescriptorSet* Device::create_descriptor_set(
//     DescriptorSetLayout& layout,
//     std::span<DescriptorSetItem> descriptors) {
//     auto& d3d12_layout = static_cast<D3d12DescriptorSetLayout&>(layout);
//
//     for (auto& layout_item : d3d12_layout.items) {
//         switch (layout_item.type) {
//         // case DescriptorType::BUFFER: create_srv();
//         default: throw std::invalid_argument("Invalid descriptor type");
//         }
//     }
//
//     throw std::invalid_argument("Not implemented");
// }

// auto Device::create_swap_chain(
//     unsigned int width,
//     unsigned int height,
//     Format format,
//     unsigned int buffer_count,
//     AppWindow& window) -> SwapChain {
//     if (window.type() != AppWindow::Type::WINDOWS) {
//         throw std::runtime_error("Invalid AppWindow type");
//     }
//     auto hwnd = reinterpret_cast<WinAppWindow&>(window).hwnd();
//
//     auto swap_chain_desc = DXGI_SWAP_CHAIN_DESC1{
//         .Width = width,
//         .Height = height,
//         .Format = to_d3d12_format(format),
//         .Stereo = false,
//         .SampleDesc = { 1, 0 },
//         .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
//         .BufferCount = buffer_count,
//         .Scaling = DXGI_SCALING_NONE,
//         .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
//         .AlphaMode = DXGI_ALPHA_MODE_IGNORE,
//         .Flags = 0
//     };
//
//     auto swap_chain_1 = static_cast<IDXGISwapChain1*>(nullptr);
//     auto swap_chain_3 = static_cast<IDXGISwapChain3*>(nullptr);
//
//     if (dxgi_factory->CreateSwapChainForHwnd(d3d12_device.Get(),
//             hwnd,
//             &swap_chain_desc,
//             nullptr,
//             nullptr,
//             &swap_chain_1)
//         != S_OK) {
//         throw std::runtime_error("Failed to create dxgi swap chain");
//     }
//     if (swap_chain_1->QueryInterface(__uuidof(IDXGISwapChain3), reinterpret_cast<void**>(&swap_chain_3)) != S_OK) {
//         throw std::runtime_error("Failed to get dxgi swap chain interface");
//     }
//
//     // TODO: Finish
//
//     throw std::runtime_error("Not implemented");
// }

// void auto Device::create_srv(DescriptorSetItem desc, D3D12_CPU_DESCRIPTOR_HANDLE dest) {
//     auto srv_desc = D3D12_SHADER_RESOURCE_VIEW_DESC{
//         .Format = to_d3d12_format(desc.format),
//         .ViewDimension = to_d3d12_srv_dimension(desc.type)
//     };
//
//     switch (desc.type) {
//     case DescriptorType::BUFFER: {
//         auto& buffer_desc = std::get<DescriptorBufferDesc>(desc.sub_desc);
//
//         srv_desc.Buffer.Flags = buffer_desc.raw ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
//         srv_desc.Buffer.FirstElement = buffer_desc.offset;
//         srv_desc.Buffer.NumElements = buffer_desc.size;
//         srv_desc.Buffer.StructureByteStride = buffer_desc.stride;
//         break;
//     }
//     case DescriptorType::TEXTURE: {
//         auto& texture_desc = std::get<DescriptorTextureDesc>(desc.sub_desc);
//         break;
//
//         // srv_desc.Te FUCK
//     }
//     }
//
//     // d3d_device->CreateShaderResourceView(resource, &srv_desc, dest);
// }
}
