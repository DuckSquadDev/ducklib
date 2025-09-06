#include <d3d12.h>

#include "../../../include/render/rhi/rhi.h"

#include "render/render_util.h"

namespace ducklib::render {
auto create_rhi(Rhi& out_rhi) -> void {
#ifdef _DEBUG
    // <OLD>
    // if (DXGIGetDebugInterface(IID_PPV_ARGS(&out_rhi.dxgi_debug)) != S_OK) {
    //     std::abort();
    // }
    //
    // if (out_rhi.dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL) != S_OK) {
    //     std::abort();
    // }
    // </OLD>

    // <NEW>
    // auto module = LoadLibrary("dxgi.dll");
    // auto get_debug_interface = reinterpret_cast<HRESULT(WINAPI*)(REFIID, void**)>(GetProcAddress(module, "DXGIGetDebugInterface1"));
    //
    // DL_CHECK_D3D(get_debug_interface(IID_PPV_ARGS(&out_rhi.dxgi_debug)));
    // DL_CHECK_D3D(out_rhi.dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL));
    // DL_CHECK_D3D(get_debug_interface(IID_PPV_ARGS(&out_rhi.dxgi_info_queue)));
    // DL_CHECK_D3D(out_rhi.dxgi_info_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE));
    // </NEW>

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
        out_adapters[i].dxgi_adapter = ComPtr<IDXGIAdapter1>(current_adapter);
        WideCharToMultiByte(CP_ACP, 0, adapter_desc.Description, -1, out_adapters[i].name, sizeof(out_adapters[i]), nullptr, nullptr);

        ++i;
    }

    return i;
}

void Rhi::create_device(const Adapter& adapter, Device& out_device) {
    DL_CHECK_D3D(
        D3D12CreateDevice(adapter.dxgi_adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(out_device.d3d12_device.GetAddressOf())));
}

void Rhi::create_swap_chain(
    const CommandQueue& command_queue,
    uint32_t width,
    uint32_t height,
    Format format,
    HWND window_handle,
    SwapChain& out_swap_chain) {
    DXGI_SWAP_CHAIN_DESC1 desc = {};

    desc.Format = to_d3d12_format(format);
    desc.Width = width;
    desc.Height = height;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.SampleDesc.Count = 1;

    DL_CHECK_D3D(
        dxgi_factory->CreateSwapChainForHwnd(
            command_queue.d3d12_queue.Get(),
            window_handle,
            &desc,
            nullptr,
            nullptr,
            &out_swap_chain.d3d12_swap_chain));
}
}
