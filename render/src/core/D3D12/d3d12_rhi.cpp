#include <d3d12.h>
#include <stdexcept>

#include "render/core/d3d12/d3d12_adapter.h"
#include "render/core/d3d12/d3d12_rhi.h"

namespace ducklib::render {
D3d12Rhi::D3d12Rhi() {
#ifdef _DEBUG
    if (DXGIGetDebugInterface(IID_PPV_ARGS(&dxgi_debug)) != S_OK) {
        throw std::runtime_error("Failed to create DXGI debug object");
    }

    if (dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL) != S_OK) {
        throw std::runtime_error("Failed to activate DXGI live object report");
    }

    if (D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12_debug)) != S_OK) {
        throw std::runtime_error("Failed to create D3D12 debug object");
    }

    d3d12_debug->EnableDebugLayer();
#endif

    if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory)) != S_OK) {
        throw std::runtime_error("Failed to create DXGI factory");
    }
}

D3d12Rhi::~D3d12Rhi() {
#ifdef _DEBUG
    if (d3d12_debug != nullptr) {
        d3d12_debug->Release();
    }

    if (dxgi_debug != nullptr) {
        dxgi_debug->Release();
    }
#endif

    if (dxgi_factory != nullptr) {
        dxgi_factory->Release();
    }
}

std::vector<std::shared_ptr<Adapter>> D3d12Rhi::enumerate_adapters() {
    std::vector<std::shared_ptr<Adapter>> adapters;
    uint32_t i = 0;
    ComPtr<IDXGIAdapter1> current_adapter = nullptr;
    DXGI_ADAPTER_DESC1 adapter_desc{};

    while (dxgi_factory->EnumAdapters1(i, &current_adapter) != DXGI_ERROR_NOT_FOUND) {
        if (current_adapter->GetDesc1(&adapter_desc) != S_OK) {
            throw std::runtime_error("Failed to get DXGI adapter description");
        }

        adapters.push_back(std::make_shared<D3d12Adapter>(current_adapter));
        ++i;
    }

    return adapters;
}

auto D3d12Rhi::create_device(Adapter* adapter) -> std::unique_ptr<Device> override {
    ID3D12Device* d3d12_device = nullptr;
    auto dxgi_adapter = reinterpret_cast<D3d12Adapter*>(adapter)->dxgi_adapter_handle();

    if (D3D12CreateDevice(dxgi_adapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&d3d12_device)) != S_OK) {
        throw std::runtime_error("Failed to create D3D12 device");
    }

    return std::make_unique<D3d12Device>(d3d12_device, dxgi_factory);
}
}
