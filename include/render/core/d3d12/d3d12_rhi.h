#ifndef D3D12_RHI_H
#define D3D12_RHI_H
#include <d3d12sdklayers.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <dxgidebug.h>

#include "d3d12_device.h"
#include "render/core/rhi.h"

using Microsoft::WRL::ComPtr;

namespace ducklib::render {
class D3d12Rhi final : public Rhi {
public:
    D3d12Rhi();
    ~D3d12Rhi() override;

    auto enumerate_adapters() -> std::vector<std::shared_ptr<Adapter>> override;
    auto create_device(Adapter* adapter) -> Device* override;

private:
#ifdef _DEBUG
    ComPtr<ID3D12Debug> d3d12_debug;
    ComPtr<IDXGIDebug> dxgi_debug;
#endif
    ComPtr<IDXGIFactory4> dxgi_factory;
};
}

#endif //D3D12_RHI_H
