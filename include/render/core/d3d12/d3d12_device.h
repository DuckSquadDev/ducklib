#ifndef D3D12_DEVICE_H
#define D3D12_DEVICE_H
#include <d3d12.h>
#include <dxgi1_5.h>
#include <vector>

#include "d3d12_command_queue.h"
#include "render/core/device.h"
#include "render/core/format.h"
#include "resources/d3d12_descriptor_set.h"

using Microsoft::WRL::ComPtr;

namespace ducklib::render {
class D3d12Device final : public Device {
public:
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
    D3d12Device(ComPtr<ID3D12Device5> d3d12_device, ComPtr<IDXGIFactory4> dxgi_factory);
    
    ComPtr<ID3D12Device> d3d12_device;
    ComPtr<IDXGIFactory4> dxgi_factory;

    auto create_srv(DescriptorSetItem desc, D3D12_CPU_DESCRIPTOR_HANDLE dest) -> void;

    auto destroy_descriptor_set_layout() -> void;

    // auto allocate_descriptor() -> D3D12_DESCRIPTOR
    std::vector<D3d12DescriptorSetLayout*> descriptor_set_layouts;
    std::vector<D3d12DescriptorSet*> descriptor_sets;

    auto create_queue(QueueType type) -> D3d12CommandQueue;
};
}

#endif //D3D12_DEVICE_H
