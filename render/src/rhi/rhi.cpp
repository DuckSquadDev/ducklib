#include <d3d12.h>

#include "render/rhi/rhi.h"
#include "render/render_util.h"

namespace ducklib::render {
auto create_rhi(Rhi& out_rhi) -> void {
#ifndef NDEBUG
    IDXGIDebug* dxgi_debug = nullptr;
    if (DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgi_debug)) != S_OK) {
        std::abort();
    }

    if (dxgi_debug->QueryInterface(__uuidof(IDXGIDebug1), &out_rhi.dxgi_debug) != S_OK) {
        std::abort();
    }

    dxgi_debug->Release();
    
    if (out_rhi.dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL) != S_OK) {
        std::abort();
    }

    DL_CHECK_D3D(D3D12GetDebugInterface(IID_PPV_ARGS(&out_rhi.d3d12_debug)));
    out_rhi.d3d12_debug->EnableDebugLayer();
#endif

    DL_CHECK_D3D(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&out_rhi.dxgi_factory)));
}

auto Rhi::enumerate_adapters(Adapter* out_adapters, uint32_t max_adapter_count) -> uint32_t {
    uint32_t i = 0;
    IDXGIAdapter1* current_adapter = nullptr;
    DXGI_ADAPTER_DESC1 adapter_desc = {};

    while (dxgi_factory->EnumAdapters1(i, &current_adapter) != DXGI_ERROR_NOT_FOUND && i < max_adapter_count) {
        DL_CHECK_D3D_THROW(current_adapter->GetDesc1(&adapter_desc));
        out_adapters[i].dxgi_adapter.Attach(current_adapter);
        current_adapter->Release();
        WideCharToMultiByte(CP_ACP, 0, adapter_desc.Description, -1, out_adapters[i].name, sizeof(out_adapters[i]), nullptr, nullptr);

        ++i;
    }

    return i;
}

void Rhi::create_device(const Adapter& adapter, Device& out_device) {
    DL_CHECK_D3D(
        D3D12CreateDevice(adapter.dxgi_adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(out_device.d3d12_device.GetAddressOf())));
    
    out_device.create_queue(QueueType::GRAPHICS, out_device.graphics_queue);
    out_device.create_queue(QueueType::COPY, out_device.copy_queue);
}

void Rhi::create_swap_chain(
    Device& device,
    DescriptorHeap& descriptor_heap,
    uint8_t buffer_count,
    uint32_t width,
    uint32_t height,
    Format format,
    HWND window_handle,
    SwapChain& out_swap_chain) {

    if (descriptor_heap.type != DescriptorHeapType::RT) {
        throw std::runtime_error("Incorrect descriptor heap type provided to swap chain creation");
    }
    
    DXGI_SWAP_CHAIN_DESC1 desc = {};

    desc.Format = to_d3d12_format(format);
    desc.Width = width;
    desc.Height = height;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = buffer_count;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.SampleDesc.Count = 1;

    DL_CHECK_D3D(
        dxgi_factory->CreateSwapChainForHwnd(
            device.graphics_queue.d3d12_queue.Get(),
            window_handle,
            &desc,
            nullptr,
            nullptr,
            &out_swap_chain.d3d12_swap_chain));

    ID3D12Resource1* buffer;
    
    for (auto i = 0; i < buffer_count; ++i) {
        out_swap_chain.d3d12_swap_chain->GetBuffer(i, IID_PPV_ARGS(&buffer));
        out_swap_chain.buffers[i] = { buffer, width, height, format };
        out_swap_chain.buffer_descriptors[i] = descriptor_heap.allocate();
        device.create_rt_descriptor(buffer, nullptr, out_swap_chain.buffer_descriptors[i]);
        buffer->Release();
    }

    out_swap_chain.buffer_count = buffer_count;
}
}
