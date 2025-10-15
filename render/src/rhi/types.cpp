#include "render/rhi/types.h"

#include "render/rhi/rhi.h"

namespace ducklib::render {
void SwapChain::present() {
    if (FAILED(d3d12_swap_chain->Present(0, 0))) {
        std::abort();
    }
}

void Fence::set_completion_value(uint64_t value) {
    if (FAILED(d3d12_fence->SetEventOnCompletion(value, event_handle))) {
        std::abort();
    }
}

void Fence::wait() {
    WaitForSingleObject(event_handle, INFINITE);
}

void CommandList::close() {
    if (FAILED(d3d12_list->Close())) {
        std::abort();
    }
}

void CommandList::reset(const Pso* pso) {
    if (FAILED(d3d12_alloc->Reset())) {
        std::abort();
    }

    ID3D12PipelineState* d3d12_pso = pso != nullptr ? pso->d3d12_pso.Get() : nullptr;
    if (FAILED(d3d12_list->Reset(d3d12_alloc.Get(), d3d12_pso))) {
        std::abort();
    }
}

void CommandList::set_pso(const Pso& pso) {
    d3d12_list->SetPipelineState(pso.d3d12_pso.Get());
}

void CommandList::set_binding_set(const BindingSet& binding_set) {
    d3d12_list->SetGraphicsRootSignature(binding_set.d3d12_signature.Get());
}

void CommandList::set_descriptor_heaps(uint32_t heap_count, DescriptorHeap** heaps) {
    assert(heap_count <= 2);
    ID3D12DescriptorHeap* d3d12_heaps[2];

    for (auto i = 0; i < heap_count; ++i) {
        d3d12_heaps[i] = heaps[i]->d3d12_heap.Get();
    }
    
    d3d12_list->SetDescriptorHeaps(heap_count, d3d12_heaps);
}

void CommandList::set_descriptor_set(uint32_t slot, Descriptor base_descriptor) {
    d3d12_list->SetGraphicsRootDescriptorTable(slot, base_descriptor.gpu_handle);
}

void CommandList::set_viewport(float top_left_x, float top_left_y, float width, float height) {
    D3D12_VIEWPORT viewport_desc = {};

    viewport_desc.Width = width;
    viewport_desc.Height = height;
    viewport_desc.MinDepth = 0.0f;
    viewport_desc.MaxDepth = 1.0f;
    viewport_desc.TopLeftX = top_left_x;
    viewport_desc.TopLeftY = top_left_y;

    d3d12_list->RSSetViewports(1, &viewport_desc);
}

void CommandList::set_scissor_rect(int32_t left, int32_t top, int32_t right, int32_t bottom) {
    D3D12_RECT rect = { left, top, right, bottom };
    d3d12_list->RSSetScissorRects(1, &rect);
}

void CommandList::set_primitive_topology(PrimitiveTopology topology) {
    d3d12_list->IASetPrimitiveTopology(to_d3d_primitive_topology(topology));
}

void CommandList::set_constant_buffer(uint32_t param_index, Buffer constant_buffer) {
    d3d12_list->SetGraphicsRootConstantBufferView(param_index, constant_buffer.d3d12_resource->GetGPUVirtualAddress());
}

void CommandList::set_vertex_buffer(const Buffer& vertex_buffer, uint32_t stride) {
    D3D12_VERTEX_BUFFER_VIEW vb_view = {};
    
    vb_view.BufferLocation = vertex_buffer.d3d12_resource->GetGPUVirtualAddress(),
    vb_view.SizeInBytes = static_cast<uint32_t>(vertex_buffer.size);
    vb_view.StrideInBytes = stride;

    d3d12_list->IASetVertexBuffers(0, 1, &vb_view);
}

void CommandList::draw(uint32_t vertex_count, uint32_t offset) {
    d3d12_list->DrawInstanced(vertex_count, 1, offset, 0);
}

void CommandList::copy_texture(Texture& dest, uint32_t dest_x, uint32_t dest_y, uint32_t dest_z, const Texture& source) {
    D3D12_TEXTURE_COPY_LOCATION dest_desc = {};
    D3D12_TEXTURE_COPY_LOCATION source_desc = {};

    dest_desc.pResource = dest.d3d12_resource.Get();
    dest_desc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dest_desc.SubresourceIndex = 0;
    source_desc.pResource = source.d3d12_resource.Get();
    source_desc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    source_desc.SubresourceIndex = 0;
    
    d3d12_list->CopyTextureRegion(&dest_desc, dest_x, dest_y, dest_z, &source_desc, nullptr);
}

void CommandList::copy_texture(Texture& dest, uint32_t dest_x, uint32_t dest_y, const Buffer& source) {
    ComPtr<ID3D12Device2> d3d12_device;
    D3D12_TEXTURE_COPY_LOCATION dest_desc = {};
    D3D12_TEXTURE_COPY_LOCATION source_desc = {};
    D3D12_RESOURCE_DESC dest_resource_desc = dest.d3d12_resource->GetDesc();
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT upload_buffer_footprint = {};
    uint32_t row_count;
    uint64_t row_size;
    uint64_t total_bytes;

    DL_CHECK_D3D(d3d12_list->GetDevice(IID_PPV_ARGS(&d3d12_device)));

    d3d12_device->GetCopyableFootprints(&dest_resource_desc, 0, 1, 0, &upload_buffer_footprint, &row_count, &row_size, &total_bytes);

    dest_desc.pResource = dest.d3d12_resource.Get();
    dest_desc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dest_desc.SubresourceIndex = 0;
    source_desc.pResource = source.d3d12_resource.Get();
    source_desc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    source_desc.PlacedFootprint = upload_buffer_footprint;

    d3d12_list->CopyTextureRegion(&dest_desc, dest_x, dest_y, 0, &source_desc, nullptr);
}

void CommandList::set_rt(const Descriptor& rt_descriptor) {
    d3d12_list->OMSetRenderTargets(1, &rt_descriptor.cpu_handle, false, nullptr);
}

void CommandList::clear_rt(const Descriptor& rt_descriptor, const float color[4]) {
    d3d12_list->ClearRenderTargetView(rt_descriptor.cpu_handle, color, 0, nullptr);
}

void CommandList::resource_barrier(void* d3d12_resource, ResourceState start_state, ResourceState end_state) {
    D3D12_RESOURCE_BARRIER barrier_desc = {};

    barrier_desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier_desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier_desc.Transition.pResource = static_cast<ID3D12Resource*>(d3d12_resource);
    barrier_desc.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(start_state);
    barrier_desc.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(end_state);
    barrier_desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    d3d12_list->ResourceBarrier(1, &barrier_desc);
}

void CommandQueue::signal(const Fence& fence, uint64_t value) {
    if (FAILED(d3d12_queue->Signal(fence.d3d12_fence.Get(), value))) {
        std::abort();
    }
}

void CommandQueue::execute(const CommandList& list) {
    ID3D12CommandList* d3d12_lists[] = { list.d3d12_list.Get() };
    d3d12_queue->ExecuteCommandLists(std::size(d3d12_lists), d3d12_lists);
}
}
