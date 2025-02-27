#include <wrl/client.h>
#include <stdexcept>

#include "core/win/win_app_window.h"
#include "render/rhi/device.h"
#include "render/rhi/command_list.h"

namespace ducklib::render {
auto Device::create_queue(QueueType type, CommandQueue* out_queue) {
    auto queue_desc = D3D12_COMMAND_QUEUE_DESC{
        .Type = to_d3d12_queue_type(type),
        .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0
    };

    if (d3d12_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&out_queue->d3d12_queue)) != S_OK) {
        std::abort();
    }
}

auto Device::create_command_list(QueueType queue_type, CommandList* out_list) {
    out_list->type = queue_type;
    auto d3d12_type = to_d3d12_queue_type(queue_type);

    if (d3d12_device->CreateCommandAllocator(d3d12_type, IID_PPV_ARGS(&out_list->d3d12_alloc)) != S_OK) {
        throw std::runtime_error("failed to create D3D12 command allocator");
    }

    if (d3d12_device->CreateCommandList(0, d3d12_type, out_list->d3d12_alloc, nullptr, IID_PPV_ARGS(&out_list->d3d12_list)) !=
        S_OK) {
        throw std::runtime_error("failed to create D3D12 command list");
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

// auto Device::create_srv(DescriptorSetItem desc, D3D12_CPU_DESCRIPTOR_HANDLE dest) -> void {
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
