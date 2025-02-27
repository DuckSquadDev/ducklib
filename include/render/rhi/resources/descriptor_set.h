#ifndef DESCRIPTOR_SET_H
#define DESCRIPTOR_SET_H
#include <variant>

#include "../constants.h"

namespace ducklib::render {
struct DescriptorSetLayoutItem {
    uint32_t slot;
    uint32_t descriptor_count;
    uint32_t stage_flags;
    DescriptorType type;
    // immutable samplers?
};

struct DescriptorBufferDesc {
    uint32_t offset;
    uint32_t size;
    uint32_t stride; // TODO: Check if this will work
    bool raw;
};

struct DescriptorTextureDesc {
    uint32_t mip_level_count;
    uint32_t base_mip_level;
    uint32_t plane_slice;
};

struct DescriptorStoreImageDesc {
    uint32_t mip_slice;
    uint32_t plane_slice;
    uint32_t base_w_level;
    uint32_t w_size;
};

struct DescriptorSetItem {
    std::variant<DescriptorBufferDesc, DescriptorTextureDesc, DescriptorStoreImageDesc> sub_desc;
    void* resource; // TODO: Fix type
    uint32_t array_size;
    Format format;
    DescriptorType type;
};

class DescriptorSetLayout {};
class DescriptorSet {};
}

#endif //DESCRIPTOR_SET_H
