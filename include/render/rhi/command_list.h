#ifndef COMMAND_LIST_H
#define COMMAND_LIST_H

#include <d3d12.h>

#include "constants.h"

namespace ducklib::render {
struct CommandList {
    ID3D12CommandAllocator* d3d12_alloc;
    ID3D12GraphicsCommandList2* d3d12_list;
    QueueType type;

    void close();
    void reset();
};
}

#endif //COMMAND_LIST_H
