#include "render/resource_manager.h"

namespace ducklib::render {
CommandList resource_list;
Buffer upload_buffer;

void setup_management_resources(Device* device) {
    device->create_command_list(QueueType::GRAPHICS, &resource_list);
    device->create_buffer(4096, &upload_buffer, HeapType::UPLOAD);
}

auto upload_buffer_data(const Buffer* dest_resource, uint64_t offset, const void* data, uint64_t size) -> uint64_t {
    void* mapped_ptr = nullptr;
    
    if (FAILED(dest_resource->d3d12_resource->Map(0, nullptr, &mapped_ptr))) {
        std::abort();
    }

    memcpy(mapped_ptr, static_cast<const char*>(data) + offset, size);
    dest_resource->d3d12_resource->Unmap(0, nullptr);

    return 0;
}
}
