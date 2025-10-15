#ifndef D3D12_RHI_H
#define D3D12_RHI_H
#ifdef _DEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif
#include <dxgi1_4.h>
#include <wrl/client.h>

#include "device.h"
#include "types.h"

using Microsoft::WRL::ComPtr;

namespace ducklib::render {
struct Rhi;
auto create_rhi(Rhi& out_rhi) -> void;

struct Rhi {
    ComPtr<IDXGIFactory4> dxgi_factory;
#ifndef NDEBUG
    ComPtr<ID3D12Debug> d3d12_debug;
    ComPtr<IDXGIDebug1> dxgi_debug;
    ComPtr<IDXGIInfoQueue> dxgi_info_queue;
#endif

    auto enumerate_adapters(Adapter* out_adapters, uint32_t max_adapter_count) -> uint32_t;
    void create_device(const Adapter& adapter, Device& out_device);
    void create_swap_chain(
        Device& device,
        DescriptorHeap& descriptor_heap,
        uint8_t buffer_count,
        uint32_t width,
        uint32_t height,
        Format format,
        HWND window_handle,
        SwapChain& out_swap_chain);
};
}

#endif //D3D12_RHI_H
