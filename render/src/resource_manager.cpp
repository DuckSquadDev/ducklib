#include "render/resource_manager.h"

#include "render/render_util.h"

namespace ducklib::render {
CommandList resource_list;
Buffer upload_buffer;

void setup_management_resources(Device* device) {
    device->create_command_list(QueueType::GRAPHICS, resource_list);
    device->create_buffer(4096, upload_buffer, HeapType::UPLOAD);
}

void upload_buffer_data(const Buffer& dest_resource, uint64_t offset, const void* data, uint64_t size) {
    void* mapped_ptr = nullptr;
    
    DL_CHECK_D3D(dest_resource.d3d12_resource->Map(0, nullptr, &mapped_ptr));

    memcpy(static_cast<std::byte*>(mapped_ptr) + offset, data, size);
    dest_resource.d3d12_resource->Unmap(0, nullptr);
}
}
