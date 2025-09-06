#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H
#include "rhi/command_list.h"
#include "rhi/device.h"
#include "rhi/types.h"

namespace ducklib::render {
extern CommandList resource_list;
extern Buffer upload_buffer;

void setup_management_resources(Device* device);
void upload_buffer_data(const Buffer* dest_resource, uint64_t offset, const void* data, uint64_t size);
}

#endif //RESOURCE_MANAGER_H
