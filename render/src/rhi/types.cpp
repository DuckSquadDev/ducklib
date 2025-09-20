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

    ID3D12PipelineState* d3d12_pso = pso != nullptr ? pso->d3d12_pso : nullptr;
    if (FAILED(d3d12_list->Reset(d3d12_alloc, d3d12_pso))) {
        std::abort();
    }
}

void CommandList::set_pso(const Pso& pso) {
    d3d12_list->SetPipelineState(pso.d3d12_pso);
}

void CommandList::set_binding_set(const BindingSet& binding_set) {
    d3d12_list->SetGraphicsRootSignature(binding_set.d3d12_signature);
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
    barrier_desc.Transition.StateBefore = to_d3d12_resource_state(start_state);
    barrier_desc.Transition.StateAfter = to_d3d12_resource_state(end_state);
    barrier_desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    d3d12_list->ResourceBarrier(1, &barrier_desc);
}

void CommandQueue::signal(const Fence& fence, uint64_t value) {
    if (FAILED(d3d12_queue->Signal(fence.d3d12_fence, value))) {
        std::abort();
    }
}

void CommandQueue::execute(const CommandList& list) {
    ID3D12CommandList* d3d12_lists[] = { list.d3d12_list };
    d3d12_queue->ExecuteCommandLists(std::size(d3d12_lists), d3d12_lists);
}
}
