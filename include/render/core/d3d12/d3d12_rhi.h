#ifndef D3D12_RHI_H
#define D3D12_RHI_H
#include "render/core/rhi.h"

namespace ducklib::render {
class D3d12Rhi : public Rhi {
public:
    D3d12Rhi();
    ~D3d12Rhi() override;
};
}

#endif //D3D12_RHI_H
