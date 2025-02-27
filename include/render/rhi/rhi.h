#ifndef D3D12_RHI_H
#define D3D12_RHI_H
#include <d3d12sdklayers.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <dxgidebug.h>

#include "device.h"
#include "types.h"

using Microsoft::WRL::ComPtr;

namespace ducklib::render {
struct Rhi;
auto create_rhi(Rhi* out_rhi) -> void;

struct Rhi {
    ComPtr<IDXGIFactory4> dxgi_factory;
#ifdef _DEBUG
    ComPtr<ID3D12Debug> d3d12_debug;
    // ComPtr<IDXGIDebug> dxgi_debug;
#endif
    
    auto enumerate_adapters(Adapter* out_adapters, uint32_t max_adapter_count) -> uint32_t;
    auto create_device(Adapter* adapter, Device* out_device) -> void;
};
}

#endif //D3D12_RHI_H