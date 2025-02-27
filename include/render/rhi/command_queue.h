#ifndef D3D12_COMMAND_QUEUE_H
#define D3D12_COMMAND_QUEUE_H
#include <d3d12.h>
#include <wrl/client.h>

#include "constants.h"

using Microsoft::WRL::ComPtr;

namespace ducklib::render {
struct CommandQueue {
    QueueType type;
    ComPtr<ID3D12CommandQueue> d3d12_queue;
};
}

#endif //D3D12_COMMAND_QUEUE_H
