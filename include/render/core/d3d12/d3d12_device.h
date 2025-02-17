#ifndef D3D12_DEVICE_H
#define D3D12_DEVICE_H
#include <d3d12.h>
#include <dxgi1_5.h>
#include <vector>

#include "render/core/device.h"
#include "render/core/format.h"
#include "resources/d3d12_descriptor_set.h"

namespace ducklib::render {
class D3d12Device final : public Device {
public:
    D3d12Device();
    ~D3d12Device() override;

    auto create_descriptor_set_layout(std::span<DescriptorSetLayoutItem> layout_items) -> DescriptorSetLayout* override;
    auto create_descriptor_set(DescriptorSetLayout& layout, std::span<DescriptorSetItem> descriptors) -> DescriptorSet* override;

    auto create_swap_chain(
        unsigned int width,
        unsigned int height,
        Format format,
        unsigned int buffer_count,
        AppWindow& window) -> SwapChain override;

private:
    ID3D12Device* d3d_device;
    IDXGIFactory4* dxgi_factory;

    auto create_srv(DescriptorSetItem desc, D3D12_CPU_DESCRIPTOR_HANDLE dest) -> void;

    // auto allocate_descriptor() -> D3D12_DESCRIPTOR
    std::vector<std::unique_ptr<D3d12DescriptorSetLayout>> descriptor_set_layouts;
    std::vector<std::unique_ptr<D3d12DescriptorSet>> descriptor_sets;
};
}

#endif //D3D12_DEVICE_H
