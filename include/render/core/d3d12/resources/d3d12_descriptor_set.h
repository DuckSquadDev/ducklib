#ifndef D3D12_DESCRIPTOR_LAYOUT_H
#define D3D12_DESCRIPTOR_LAYOUT_H
#include <memory>

#include "render/core/descriptor_set.h"

namespace ducklib::render {
class D3d12DescriptorSetLayout : DescriptorSetLayout {
protected:
    explicit D3d12DescriptorSetLayout(std::span<DescriptorSetLayoutItem> items)
        : items(items.begin(), items.end()) {}

private:
    std::vector<DescriptorSetLayoutItem> items;

    friend class D3d12Device;
};

class D3d12DescriptorSet : DescriptorSet {
protected:

private:
    friend class D3d12Device;
};
}

#endif //D3D12_DESCRIPTOR_LAYOUT_H
