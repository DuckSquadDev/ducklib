#ifndef RHI_H
#define RHI_H
#include "adapter.h"

namespace ducklib::render {
class Rhi {
public:
    virtual ~Rhi();

    virtual auto enumerate_adapters() -> std::vector<std::shared_ptr<Adapter>>;
    virtual auto create_device(Adapter* adapter) -> std::unique_ptr<Device>;
};
}

#endif //RHI_H
