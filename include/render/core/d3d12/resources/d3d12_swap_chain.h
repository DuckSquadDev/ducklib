#ifndef D3D12_SWAP_CHAIN_H
#define D3D12_SWAP_CHAIN_H
#include <d3d12.h>
#include <dxgi1_2.h>

#include "render/core/resources/swap_chain.h"

namespace ducklib::render {
class D3d12SwapChain : public SwapChain {
    ID3D12Resource* resource;
    IDXGISwapChain1* swap_chain;
};
}

#endif //D3D12_SWAP_CHAIN_H
