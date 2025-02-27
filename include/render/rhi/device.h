#ifndef D3D12_DEVICE_H
#define D3D12_DEVICE_H
#include <d3d12.h>
#include <dxgi1_5.h>

#include "command_queue.h"
#include "command_list.h"
#include "constants.h"
#include "resources/descriptor_set.h"

using Microsoft::WRL::ComPtr;

namespace ducklib::render {
struct Device {
    ComPtr<ID3D12Device> d3d12_device;
    
    auto create_queue(QueueType type, CommandQueue* out_queue);
    auto create_command_list(QueueType queue_type, CommandList* out_list);

    // auto create_vertex_buffer() -> VertexBuffer*;

    auto create_descriptor_set_layout(DescriptorSetLayoutItem* layout_items, uint32_t item_count) -> DescriptorSetLayout*;
    auto create_descriptor_set(DescriptorSetLayout* layout, DescriptorSetItem* descriptors, uint32_t descriptor_count) -> DescriptorSet*;
};
}

#endif //D3D12_DEVICE_H
