#include <d3d12.h>
#include <stdexcept>

#include "../../../include/render/rhi/rhi.h"

namespace ducklib::render {
auto create_rhi(Rhi* out_rhi) -> void {
#ifdef _DEBUG
    // if (DXGIGetDebugInterface(IID_PPV_ARGS(&out_rhi->dxgi_debug)) != S_OK) {
    //     std::abort();
    // }
    //
    // if (out_rhi->dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL) != S_OK) {
    //     std::abort();
    // }

    if (D3D12GetDebugInterface(IID_PPV_ARGS(&out_rhi->d3d12_debug)) != S_OK) {
        std::abort();
    }

    out_rhi->d3d12_debug->EnableDebugLayer();
#endif

    if (CreateDXGIFactory1(IID_PPV_ARGS(&out_rhi->dxgi_factory)) != S_OK) {
        std::abort();
    }
}

auto Rhi::enumerate_adapters(Adapter* out_adapters, uint32_t max_adapter_count) -> uint32_t {
    uint32_t i = 0;
    IDXGIAdapter1* current_adapter = nullptr;
    DXGI_ADAPTER_DESC1 adapter_desc = {};

    while (dxgi_factory->EnumAdapters1(i, &current_adapter) != DXGI_ERROR_NOT_FOUND && i < max_adapter_count) {
        if (current_adapter->GetDesc1(&adapter_desc) != S_OK) {
            std::abort();
        }

        out_adapters[i].dxgi_adapter = ComPtr<IDXGIAdapter1>(current_adapter);
        WideCharToMultiByte(CP_ACP, 0, adapter_desc.Description, -1, out_adapters[i].name, sizeof(out_adapters[i]), nullptr, nullptr);

        ++i;
    }

    return i;
}

auto Rhi::create_device(Adapter* adapter, Device* out_device) -> void {
    if (FAILED(
        D3D12CreateDevice(adapter->dxgi_adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(out_device->d3d12_device.GetAddressOf())))) {
        std::abort();
    }
}
}
