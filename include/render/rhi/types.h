#ifndef TYPES_H
#define TYPES_H

#include <wrl/client.h>
#include "../lib/dxgi1_4.h"

using Microsoft::WRL::ComPtr;

namespace ducklib::render {
struct Adapter {
    ComPtr<IDXGIAdapter1> dxgi_adapter = nullptr;
    char name[128] = {};
};
}

#endif //TYPES_H
