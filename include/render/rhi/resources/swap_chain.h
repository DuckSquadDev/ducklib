#ifndef D3D12_SWAP_CHAIN_H
#define D3D12_SWAP_CHAIN_H
#include <d3d12.h>
#include <dxgi1_2.h>

namespace ducklib::render {
struct SwapChain {
    // ID3D12Resource* resource;
    IDXGISwapChain1* swap_chain;
};
}

#endif //D3D12_SWAP_CHAIN_H
