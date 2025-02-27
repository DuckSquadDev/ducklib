#ifndef COMMAND_LIST_H
#define COMMAND_LIST_H

namespace ducklib::render {
struct CommandList {
    ID3D12CommandAllocator* d3d12_alloc;
    ID3D12CommandList* d3d12_list;
    QueueType type;
};
}

#endif //COMMAND_LIST_H
