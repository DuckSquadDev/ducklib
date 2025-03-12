#ifndef D3D12_DEVICE_H
#define D3D12_DEVICE_H
#include "../lib/d3dx12.h"
#include <dxgi1_5.h>

#include "command_queue.h"
#include "command_list.h"
#include "constants.h"
#include "resources/descriptor_set.h"
#include "types.h"

using Microsoft::WRL::ComPtr;

namespace ducklib::render {
struct Device {
    ComPtr<ID3D12Device2> d3d12_device;
    
    void create_queue(QueueType type, CommandQueue& out_queue);
    void create_command_list(QueueType queue_type, CommandList& out_list);

    void create_descriptor_heap(DescriptorHeapType type, uint32_t count, DescriptorHeap& out_heap);
    void create_cbuffer_descriptor(const Buffer& cbuffer, const Descriptor& descriptor);
    void create_srv_descriptor(const DescriptorDesc& desc, const Descriptor& descriptor);
    void create_uav_descriptor(const DescriptorDesc& desc, const Descriptor& descriptor);

    // Equivalent to D3D12's root signature
    void create_binding_set(const BindingSetDesc& binding_set_desc, BindingSet& out_set);
    void create_pso(const BindingSet& binding_set, const PsoDesc& pso_desc, Pso& pso_out);

    void create_buffer(uint64_t byte_size, Buffer& out_buffer, HeapType heap_type = HeapType::DEFAULT);
};
}

#endif //D3D12_DEVICE_H
