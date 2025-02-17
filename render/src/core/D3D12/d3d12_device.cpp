#include <wrl/client.h>

#include <stdexcept>

#include "core/win/win_app_window.h"
#include "render/core/d3d12/d3d12_device.h"
#include "render/core/d3d12/d3d12_format.h"
#include "render/core/d3d12/d3d12_utility.h"
#include "render/core/d3d12/resources/d3d12_descriptor_set.h"

namespace ducklib::render {
D3d12Device::~D3d12Device() {
    if (d3d_device) {
        d3d_device->Release();
    }

    if (dxgi_factory) {
        dxgi_factory->Release();
    }
}

auto D3d12Device::create_descriptor_set_layout(
    std::span<DescriptorSetLayoutItem> layout_items)
    -> DescriptorSetLayout* {
    auto layout = std::unique_ptr<D3d12DescriptorSetLayout>(new D3d12DescriptorSetLayout(layout_items));
    descriptor_set_layouts.push_back(std::move(layout));

    return layout.get();
}

DescriptorSet* D3d12Device::create_descriptor_set(
    DescriptorSetLayout& layout,
    std::span<DescriptorSetItem> descriptors) {
    auto& d3d12_layout = static_cast<D3d12DescriptorSetLayout&>(layout);

    for (auto& layout_item : d3d12_layout.items) {
        switch (layout_item.type) {
        // case DescriptorType::BUFFER: create_srv();
        default: throw std::invalid_argument("Invalid descriptor type");
        }
    }

    throw std::invalid_argument("Not implemented");
}


auto D3d12Device::create_swap_chain(
    unsigned int width,
    unsigned int height,
    Format format,
    unsigned int buffer_count,
    AppWindow& window) -> SwapChain {
    if (window.type() != AppWindow::Type::WINDOWS) {
        throw std::runtime_error("Invalid AppWindow type");
    }
    auto hwnd = reinterpret_cast<WinAppWindow&>(window).hwnd();

    auto swap_chain_desc = DXGI_SWAP_CHAIN_DESC1{
        .Width = width,
        .Height = height,
        .Format = to_d3d12_format(format),
        .Stereo = false,
        .SampleDesc = { 1, 0 },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = buffer_count,
        .Scaling = DXGI_SCALING_NONE,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        .AlphaMode = DXGI_ALPHA_MODE_IGNORE,
        .Flags = 0
    };

    auto swap_chain_1 = static_cast<IDXGISwapChain1*>(nullptr);
    auto swap_chain_3 = static_cast<IDXGISwapChain3*>(nullptr);

    if (dxgi_factory->CreateSwapChainForHwnd(d3d_device, hwnd, &swap_chain_desc, nullptr, nullptr, &swap_chain_1) !=
        S_OK) {
        throw std::runtime_error("Failed to create dxgi swap chain");
    }
    if (swap_chain_1->QueryInterface(__uuidof(IDXGISwapChain3), reinterpret_cast<void**>(&swap_chain_3)) != S_OK) {
        throw std::runtime_error("Failed to get dxgi swap chain interface");
    }

    // TODO: Finish

    throw std::runtime_error("Not implemented");
}

auto D3d12Device::create_srv(DescriptorSetItem desc, D3D12_CPU_DESCRIPTOR_HANDLE dest) -> void {
    auto srv_desc = D3D12_SHADER_RESOURCE_VIEW_DESC{
        .Format = to_d3d12_format(desc.format),
        .ViewDimension = to_d3d12_srv_dimension(desc.type)
    };

    switch (desc.type) {
    case DescriptorType::BUFFER: {
        auto& buffer_desc = std::get<DescriptorBufferDesc>(desc.sub_desc);

        srv_desc.Buffer.Flags = buffer_desc.raw ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
        srv_desc.Buffer.FirstElement = buffer_desc.offset;
        srv_desc.Buffer.NumElements = buffer_desc.size;
        srv_desc.Buffer.StructureByteStride = buffer_desc.stride;
        break;
    }
    case DescriptorType::TEXTURE: {
        auto& texture_desc = std::get<DescriptorTextureDesc>(desc.sub_desc);
        break;

        // srv_desc.Te FUCK
    }
    }

    // d3d_device->CreateShaderResourceView(resource, &srv_desc, dest);
}
}
