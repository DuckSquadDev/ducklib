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

void CommandList::reset() {
    if (FAILED(d3d12_alloc->Reset())) {
        std::abort();
    }

    if (FAILED(d3d12_list->Reset(d3d12_alloc, nullptr))) {
        std::abort();
    }
}

void CommandList::set_pso(const Pso& pso) {
    d3d12_list->SetPipelineState(pso.d3d12_pso);
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
