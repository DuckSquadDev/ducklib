#include <wrl/client.h>
#include <stdexcept>

#include "ducklib/core/win/win_app_window.h"
#include "ducklib/render/rhi/device.h"

#include "ducklib/render/render_util.h"
#include "ducklib/render/rhi/command_list.h"
#include "ducklib/render/rhi/shader.h"

namespace ducklib::render {
void Device::create_queue(QueueType type, CommandQueue& out_queue) {
    D3D12_COMMAND_QUEUE_DESC queue_desc = {};
    queue_desc.Type = to_d3d12_queue_type(type);
    queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.NodeMask = 0;

    DL_CHECK_D3D(d3d12_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&out_queue.d3d12_queue)));
}

void Device::create_command_list(QueueType queue_type, CommandList& out_list) {
    out_list.type = queue_type;
    auto d3d12_type = to_d3d12_queue_type(queue_type);

    DL_CHECK_D3D(d3d12_device->CreateCommandAllocator(d3d12_type, IID_PPV_ARGS(&out_list.d3d12_alloc)));
    DL_CHECK_D3D(d3d12_device->CreateCommandList(0, d3d12_type, out_list.d3d12_alloc.Get(), nullptr, IID_PPV_ARGS(&out_list.d3d12_list)));
    out_list.close();
}

void Device::create_fence(uint64_t initial_value, Fence& out_fence) {
    DL_CHECK_D3D(d3d12_device->CreateFence(initial_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&out_fence.d3d12_fence)));
}

void Device::create_descriptor_heap(DescriptorHeapType type, uint32_t count, DescriptorHeap& out_heap) {
    D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
    D3D12_DESCRIPTOR_HEAP_TYPE d3d12_heap_type = to_d3d12_descriptor_heap_type(type);

    heap_desc.Type = d3d12_heap_type;
    heap_desc.NumDescriptors = count;
    heap_desc.Flags = type != DescriptorHeapType::RT ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    DL_CHECK_D3D(d3d12_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&out_heap.d3d12_heap)));

    out_heap.first_cpu_handle = out_heap.d3d12_heap->GetCPUDescriptorHandleForHeapStart();
    out_heap.first_gpu_handle = out_heap.d3d12_heap->GetGPUDescriptorHandleForHeapStart();
    out_heap.descriptor_size = d3d12_device->GetDescriptorHandleIncrementSize(d3d12_heap_type);
    out_heap.type = type;
    out_heap.count = count;
}

void Device::create_cbuffer_descriptor(const Buffer& cbuffer, const Descriptor& descriptor) {
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbuffer_desc = {};

    cbuffer_desc.BufferLocation = cbuffer.d3d12_resource->GetGPUVirtualAddress();
    cbuffer_desc.SizeInBytes = cbuffer.size;

    d3d12_device->CreateConstantBufferView(&cbuffer_desc, descriptor.cpu_handle);
}

// @brief desc is optional
void Device::create_srv_descriptor(void* resource, const DescriptorDesc* desc, const Descriptor& descriptor) {
    if (desc == nullptr) {
        d3d12_device->CreateShaderResourceView(static_cast<ID3D12Resource*>(resource), nullptr, descriptor.cpu_handle);
        return;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC d3d12_desc = {};
    auto d3d12_type = to_d3d12_srv_dimension(desc->type, desc->array_size);

    d3d12_desc.Format = to_d3d12_format(desc->format);
    d3d12_desc.ViewDimension = d3d12_type;

    switch (d3d12_type) {
    case D3D12_SRV_DIMENSION_BUFFER:
        d3d12_desc.Buffer.FirstElement = desc->buffer.offset;
        d3d12_desc.Buffer.NumElements = desc->buffer.size;
        d3d12_desc.Buffer.StructureByteStride = desc->buffer.stride;
        d3d12_desc.Buffer.Flags = desc->buffer.raw ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
        break;
    case D3D12_SRV_DIMENSION_TEXTURE1D:
        d3d12_desc.Texture1D.MipLevels = desc->texture.mip_level_count;
        d3d12_desc.Texture1D.MostDetailedMip = desc->texture.base_mip_level;
        d3d12_desc.Texture1D.ResourceMinLODClamp = 0.0f;
        break;
    case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
        d3d12_desc.Texture1DArray.MipLevels = desc->texture.mip_level_count;
        d3d12_desc.Texture1DArray.MostDetailedMip = desc->texture.base_mip_level;
        d3d12_desc.Texture1DArray.ResourceMinLODClamp = 0.0f;
        d3d12_desc.Texture1DArray.ArraySize = desc->array_size;
        d3d12_desc.Texture1DArray.FirstArraySlice = desc->array_offset;
        break;
    case D3D12_SRV_DIMENSION_TEXTURE2D:
        d3d12_desc.Texture2D.MipLevels = desc->texture.mip_level_count;
        d3d12_desc.Texture2D.MostDetailedMip = desc->texture.base_mip_level;
        d3d12_desc.Texture2D.ResourceMinLODClamp = 0.0f;
        d3d12_desc.Texture2D.PlaneSlice = desc->texture.plane_slice;
        break;
    case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
        d3d12_desc.Texture2DArray.MipLevels = desc->texture.mip_level_count;
        d3d12_desc.Texture2DArray.MostDetailedMip = desc->texture.base_mip_level;
        d3d12_desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
        d3d12_desc.Texture2DArray.PlaneSlice = desc->texture.plane_slice;
        d3d12_desc.Texture2DArray.ArraySize = desc->array_size;
        d3d12_desc.Texture2DArray.FirstArraySlice = desc->array_offset;
        break;
    case D3D12_SRV_DIMENSION_TEXTURE3D:
        d3d12_desc.Texture3D.MipLevels = desc->texture.mip_level_count;
        d3d12_desc.Texture3D.MostDetailedMip = desc->texture.base_mip_level;
        d3d12_desc.Texture3D.ResourceMinLODClamp = 0.0f;
        break;
    case D3D12_SRV_DIMENSION_TEXTURECUBE:
        d3d12_desc.TextureCube.MipLevels = desc->texture.mip_level_count;
        d3d12_desc.TextureCube.MostDetailedMip = desc->texture.base_mip_level;
        d3d12_desc.TextureCube.ResourceMinLODClamp = 0.0f;
        break;
    case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
        d3d12_desc.TextureCubeArray.MipLevels = desc->texture.mip_level_count;
        d3d12_desc.TextureCubeArray.MostDetailedMip = desc->texture.base_mip_level;
        d3d12_desc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
        d3d12_desc.TextureCubeArray.NumCubes = desc->array_size;
        d3d12_desc.TextureCubeArray.First2DArrayFace = desc->array_offset;
        break;
    default: std::abort();
    }

    d3d12_device->CreateShaderResourceView(static_cast<ID3D12Resource*>(resource), &d3d12_desc, descriptor.cpu_handle);
}

void Device::create_uav_descriptor(void* resource, const DescriptorDesc* desc, const Descriptor& descriptor) {
    if (desc == nullptr) {
        d3d12_device->CreateUnorderedAccessView(static_cast<ID3D12Resource*>(resource), nullptr, nullptr, descriptor.cpu_handle);
        return;
    }

    D3D12_UNORDERED_ACCESS_VIEW_DESC d3d12_desc = {};
    auto d3d12_type = to_d3d12_uav_dimension(desc->type, desc->array_size);

    d3d12_desc.Format = to_d3d12_format(desc->format);
    d3d12_desc.ViewDimension = d3d12_type;

    switch (d3d12_type) {
    case D3D12_UAV_DIMENSION_BUFFER:
        d3d12_desc.Buffer.FirstElement = desc->buffer.offset;
        d3d12_desc.Buffer.NumElements = desc->buffer.size;
        d3d12_desc.Buffer.StructureByteStride = desc->buffer.stride;
        d3d12_desc.Buffer.CounterOffsetInBytes = 0;
        d3d12_desc.Buffer.Flags = desc->buffer.raw ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;
        break;
    case D3D12_UAV_DIMENSION_TEXTURE1D:
        d3d12_desc.Texture1D.MipSlice = desc->storage_image.mip_slice;
        break;
    case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
        d3d12_desc.Texture1DArray.MipSlice = desc->storage_image.mip_slice;
        d3d12_desc.Texture1DArray.ArraySize = desc->array_size;
        d3d12_desc.Texture1DArray.FirstArraySlice = desc->array_offset;
        break;
    case D3D12_UAV_DIMENSION_TEXTURE2D:
        d3d12_desc.Texture2D.MipSlice = desc->storage_image.mip_slice;
        d3d12_desc.Texture2D.PlaneSlice = desc->storage_image.plane_slice;
        break;
    case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
        d3d12_desc.Texture2DArray.MipSlice = desc->storage_image.mip_slice;
        d3d12_desc.Texture2DArray.PlaneSlice = desc->storage_image.plane_slice;
        d3d12_desc.Texture2DArray.ArraySize = desc->array_size;
        d3d12_desc.Texture2DArray.FirstArraySlice = desc->array_offset;
        break;
    case D3D12_UAV_DIMENSION_TEXTURE3D:
        d3d12_desc.Texture3D.MipSlice = desc->storage_image.mip_slice;
        d3d12_desc.Texture3D.FirstWSlice = desc->storage_image.base_w_level;
        d3d12_desc.Texture3D.WSize = desc->storage_image.w_size;
        break;
    default: std::abort();
    }

    d3d12_device->CreateUnorderedAccessView(static_cast<ID3D12Resource*>(resource), nullptr, &d3d12_desc, descriptor.cpu_handle);
}

void Device::create_rt_descriptor(void* resource, const DescriptorDesc* desc, const Descriptor& descriptor) {
    if (desc == nullptr) {
        d3d12_device->CreateRenderTargetView(static_cast<ID3D12Resource*>(resource), nullptr, descriptor.cpu_handle);
        return;
    }

    D3D12_RENDER_TARGET_VIEW_DESC d3d12_desc = {};
    auto d3d12_type = to_d3d12_rtv_dimension(desc->type, desc->array_size);

    d3d12_desc.Format = to_d3d12_format(desc->format);
    d3d12_desc.ViewDimension = d3d12_type;

    switch (d3d12_type) {
    case D3D12_RTV_DIMENSION_BUFFER:
        d3d12_desc.Buffer.FirstElement = desc->buffer.offset;
        d3d12_desc.Buffer.NumElements = desc->buffer.size;
        break;
    case D3D12_RTV_DIMENSION_TEXTURE1D:
        d3d12_desc.Texture1D.MipSlice = desc->storage_image.mip_slice;
        break;
    case D3D12_RTV_DIMENSION_TEXTURE1DARRAY:
        d3d12_desc.Texture1DArray.MipSlice = desc->storage_image.mip_slice;
        d3d12_desc.Texture1DArray.ArraySize = desc->array_size;
        d3d12_desc.Texture1DArray.FirstArraySlice = desc->array_offset;
        break;
    case D3D12_RTV_DIMENSION_TEXTURE2D:
        d3d12_desc.Texture2D.MipSlice = desc->storage_image.mip_slice;
        d3d12_desc.Texture2D.PlaneSlice = desc->storage_image.plane_slice;
        break;
    case D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
        d3d12_desc.Texture2DArray.MipSlice = desc->storage_image.mip_slice;
        d3d12_desc.Texture2DArray.PlaneSlice = desc->storage_image.plane_slice;
        d3d12_desc.Texture2DArray.ArraySize = desc->array_size;
        d3d12_desc.Texture2DArray.FirstArraySlice = desc->array_offset;
        break;
    case D3D12_RTV_DIMENSION_TEXTURE3D:
        d3d12_desc.Texture3D.MipSlice = desc->storage_image.mip_slice;
        d3d12_desc.Texture3D.FirstWSlice = desc->storage_image.base_w_level;
        d3d12_desc.Texture3D.WSize = desc->storage_image.w_size;
        break;
    default: std::abort();
    }

    d3d12_device->CreateRenderTargetView(static_cast<ID3D12Resource*>(resource), &d3d12_desc, descriptor.cpu_handle);
}

void Device::create_binding_set(const BindingSetDesc& binding_set_desc, BindingSet& out_set) {
    D3D12_ROOT_PARAMETER1 d3d12_params[64] = {};
    D3D12_DESCRIPTOR_RANGE1 d3d12_descriptor_ranges[64] = {};
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
            const DescriptorSetBindingDesc& set_binding = binding_set_desc.bindings[i].descriptor_set_binding;

            d3d12_params[i].ParameterType = to_d3d12_binding_type(binding_set_desc.bindings[i].type);
            d3d12_params[i].DescriptorTable.NumDescriptorRanges = 1;
            d3d12_params[i].DescriptorTable.pDescriptorRanges = &d3d12_descriptor_ranges[i_ranges];
            d3d12_descriptor_ranges[i_ranges].RangeType = to_d3d12_descriptor_range_type(set_binding.type);
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
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC signature_desc = {};

    signature_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    signature_desc.Desc_1_1.NumParameters = binding_set_desc.binding_count;
    signature_desc.Desc_1_1.pParameters = d3d12_params;
    signature_desc.Desc_1_1.NumStaticSamplers = 0;
    signature_desc.Desc_1_1.pStaticSamplers = nullptr;
    // signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
    signature_desc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    DL_CHECK_D3D(D3D12SerializeVersionedRootSignature(&signature_desc, &signature_blob, &errors));

    DL_CHECK_D3D(
        d3d12_device->CreateRootSignature(
            0,
            signature_blob->GetBufferPointer(),
            signature_blob->GetBufferSize(),
            IID_PPV_ARGS(&out_set.d3d12_signature)));
}

void Device::create_pso(const BindingSet& binding_set, const PsoDesc& pso_desc, Pso& pso_out) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12_pso_desc = {};
    D3D12_INPUT_ELEMENT_DESC d3d12_input_elements[32] = {};

    d3d12_pso_desc.pRootSignature = binding_set.d3d12_signature.Get();
    d3d12_pso_desc.VS = to_d3d12_bytecode(pso_desc.vertex_shader);
    d3d12_pso_desc.PS = to_d3d12_bytecode(pso_desc.pixel_shader);
    d3d12_pso_desc.GS = to_d3d12_bytecode(pso_desc.geometry_shader);
    d3d12_pso_desc.DS = to_d3d12_bytecode(pso_desc.domain_shader);
    d3d12_pso_desc.HS = to_d3d12_bytecode(pso_desc.hull_shader);
    d3d12_pso_desc.RasterizerState.FillMode = to_d3d12_fill_mode(pso_desc.rasterizer.fill_mode);
    d3d12_pso_desc.RasterizerState.CullMode = to_d3d12_cull_mode(pso_desc.rasterizer.cull_mode);
    d3d12_pso_desc.RasterizerState.FrontCounterClockwise = to_d3d12_front_face(pso_desc.rasterizer.front_face);
    d3d12_pso_desc.RasterizerState.DepthBias = 0;
    d3d12_pso_desc.RasterizerState.DepthBiasClamp = 0.0f;
    d3d12_pso_desc.RasterizerState.SlopeScaledDepthBias = 0.0f;
    d3d12_pso_desc.RasterizerState.DepthClipEnable = pso_desc.rasterizer.clip_depth;
    d3d12_pso_desc.RasterizerState.MultisampleEnable = pso_desc.rasterizer.msaa;
    d3d12_pso_desc.RasterizerState.AntialiasedLineEnable = false;
    d3d12_pso_desc.RasterizerState.ForcedSampleCount = 0;
    d3d12_pso_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    d3d12_pso_desc.BlendState = CD3DX12_BLEND_DESC{ D3D12_DEFAULT };

    for (uint32_t i = 0; i < pso_desc.input_layout.element_count; ++i) {
        const InputLayoutItem& input_element = pso_desc.input_layout.elements[i];
        d3d12_input_elements[i].SemanticName = input_element.semantic_name;
        d3d12_input_elements[i].SemanticIndex = input_element.semantic_index;
        d3d12_input_elements[i].InputSlot = input_element.input_slot;
        d3d12_input_elements[i].InstanceDataStepRate = input_element.instances_per_step;
        d3d12_input_elements[i].AlignedByteOffset = input_element.aligned_byte_offset;
        d3d12_input_elements[i].Format = to_d3d12_format(input_element.format);
        d3d12_input_elements[i].InputSlotClass = to_d3d12_input_slot_type(input_element.slot_type);
    }

    d3d12_pso_desc.InputLayout.NumElements = pso_desc.input_layout.element_count;
    d3d12_pso_desc.InputLayout.pInputElementDescs = d3d12_input_elements;

    d3d12_pso_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    d3d12_pso_desc.PrimitiveTopologyType = to_d3d12_primitive_topology_type(pso_desc.primitive_topology);
    d3d12_pso_desc.NumRenderTargets = pso_desc.rt_count;

    for (uint32_t i = 0; i < pso_desc.rt_count; ++i) {
        d3d12_pso_desc.RTVFormats[i] = to_d3d12_format(pso_desc.rt_formats[i]);

        d3d12_pso_desc.BlendState.RenderTarget[i].BlendEnable = pso_desc.blend.rts[i].blend_enable;
        d3d12_pso_desc.BlendState.RenderTarget[i].LogicOpEnable = pso_desc.blend.rts[i].logic_op_enable;
        d3d12_pso_desc.BlendState.RenderTarget[i].SrcBlend = to_d3d12_blend(pso_desc.blend.rts[i].source_blend);
        d3d12_pso_desc.BlendState.RenderTarget[i].DestBlend = to_d3d12_blend(pso_desc.blend.rts[i].dest_blend);
        d3d12_pso_desc.BlendState.RenderTarget[i].BlendOp = to_d3d12_blend_op(pso_desc.blend.rts[i].blend_op);
        d3d12_pso_desc.BlendState.RenderTarget[i].SrcBlendAlpha = to_d3d12_blend(pso_desc.blend.rts[i].source_blend_alpha);
        d3d12_pso_desc.BlendState.RenderTarget[i].DestBlendAlpha = to_d3d12_blend(pso_desc.blend.rts[i].dest_blend_alpha);
        d3d12_pso_desc.BlendState.RenderTarget[i].BlendOpAlpha = to_d3d12_blend_op(pso_desc.blend.rts[i].blend_op_alpha);
        d3d12_pso_desc.BlendState.RenderTarget[i].LogicOp = to_d3d12_logic_op(pso_desc.blend.rts[i].logic_op);
        d3d12_pso_desc.BlendState.RenderTarget[i].RenderTargetWriteMask = pso_desc.blend.rts[i].rt_write_mask;
    }

    d3d12_pso_desc.DSVFormat = to_d3d12_format(pso_desc.ds_format);
    d3d12_pso_desc.SampleDesc.Count = 1;
    d3d12_pso_desc.SampleDesc.Quality = 0;
    d3d12_pso_desc.SampleMask = UINT_MAX;
    d3d12_pso_desc.NodeMask = 0;
    d3d12_pso_desc.CachedPSO.pCachedBlob = nullptr;
    d3d12_pso_desc.CachedPSO.CachedBlobSizeInBytes = 0;
    d3d12_pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    DL_CHECK_D3D(d3d12_device->CreateGraphicsPipelineState(&d3d12_pso_desc, IID_PPV_ARGS(&pso_out.d3d12_pso)));
}

void Device::create_buffer(uint64_t byte_size, Buffer& out_buffer, HeapType heap_type) {
    D3D12_HEAP_PROPERTIES heap_props = {};
    D3D12_RESOURCE_DESC resource_desc = {};
    D3D12_RESOURCE_STATES init_state = {};

    if (heap_type == HeapType::DEFAULT) {
        heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
        init_state = D3D12_RESOURCE_STATE_COMMON;
    } else {
        heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;
        init_state = D3D12_RESOURCE_STATE_GENERIC_READ;
    }

    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resource_desc.Format = DXGI_FORMAT_UNKNOWN;
    resource_desc.Width = byte_size;
    resource_desc.Height = 1;
    resource_desc.DepthOrArraySize = 1;
    resource_desc.MipLevels = 1;
    resource_desc.SampleDesc.Count = 1;
    resource_desc.SampleDesc.Quality = 0;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    DL_CHECK_D3D(
        d3d12_device->CreateCommittedResource(
            &heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc,init_state, nullptr, IID_PPV_ARGS(&out_buffer.d3d12_resource)));

    out_buffer.size = byte_size;
}

void Device::create_texture(uint32_t width, uint32_t height, Format format, Texture& out_texture, HeapType heap_type) {
    D3D12_HEAP_PROPERTIES heap_props = {};
    D3D12_RESOURCE_DESC resource_desc = {};
    D3D12_RESOURCE_STATES init_state = {};

    if (heap_type == HeapType::DEFAULT) {
        heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
        init_state = D3D12_RESOURCE_STATE_COMMON;
    } else {
        heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;
        init_state = D3D12_RESOURCE_STATE_GENERIC_READ;
    }

    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resource_desc.Width = width;
    resource_desc.Height = height;
    resource_desc.Format = to_d3d12_format(format);
    resource_desc.DepthOrArraySize = 1;
    resource_desc.MipLevels = 1;
    resource_desc.SampleDesc.Count = 1;
    resource_desc.SampleDesc.Quality = 0;

    DL_CHECK_D3D(
        d3d12_device->CreateCommittedResource(
            &heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc, init_state, nullptr, IID_PPV_ARGS(&out_texture.d3d12_resource)));

    out_texture.width = width;
    out_texture.height = height;
    out_texture.format = format;
}

void Device::create_sampler(Filter filter, Descriptor descriptor) {
    D3D12_SAMPLER_DESC sampler_desc = {};

    sampler_desc.Filter = to_d3d12_filter(filter);
    sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler_desc.MinLOD = 0.0f;
    sampler_desc.MaxLOD = D3D12_FLOAT32_MAX;
    
    d3d12_device->CreateSampler(&sampler_desc, descriptor.cpu_handle);
}
}
