#ifndef DEVICE_H
#define DEVICE_H
#include <span>

#include "descriptor_set.h"
#include "format.h"
#include "swap_chain.h"
#include "core/app_window.h"

namespace ducklib::render {
class Device {
public:
    virtual ~Device() = 0;

    virtual auto create_descriptor_set_layout(std::span<DescriptorSetLayoutItem> layout_items) -> DescriptorSetLayout* = 0;
    virtual auto create_descriptor_set(DescriptorSetLayout& layout, std::span<DescriptorSetItem> descriptors) -> DescriptorSet* = 0;

    virtual auto create_swap_chain(
        unsigned int width,
        unsigned int height,
        Format format,
        unsigned int buffer_count,
        AppWindow& window) -> SwapChain = 0;
};
}

#endif //DEVICE_H
