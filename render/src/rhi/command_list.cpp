#include <cstdlib>

#include "render/rhi/command_list.h"

namespace ducklib::render {
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
}