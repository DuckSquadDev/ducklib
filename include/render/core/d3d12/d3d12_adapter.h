#ifndef D3D12_ADAPTER_H
#define D3D12_ADAPTER_H
#include <dxgi.h>
#include <wrl/client.h>

#include "render/core/adapter.h"
#include "render/core/device.h"

using Microsoft::WRL::ComPtr;

namespace ducklib::render {
class D3d12Adapter : public Adapter {
public:
    D3d12Adapter(ComPtr<IDXGIAdapter1> dxgi_adapter);
    ~D3d12Adapter() override;

    auto dxgi_adapter_handle() -> IDXGIAdapter1*;
};
}

#endif //D3D12_ADAPTER_H
