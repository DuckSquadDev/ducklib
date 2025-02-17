#ifndef RHI_H
#define RHI_H
#include "swap_chain.h"
#include "core/app_window.h"

namespace ducklib::render {
class Rhi {
public:
    virtual ~Rhi();
};
}

#endif //RHI_H
