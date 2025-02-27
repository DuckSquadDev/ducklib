#include <Windows.h>
#include <memory>

#include "core/win/win_app_window.h"
#include "render/rhi/rhi.h"

using namespace ducklib;

int __stdcall WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* cmdLine, int cmdShow) {
    auto window = std::make_unique<WinAppWindow>("Hello world!", 600, 400);
    // auto rhi = std::make_unique<render::Rhi>();
    // auto adapters = rhi->enumerate_adapters();
    // auto device = rhi->create_device(adapters[0].get());
    // auto queue = device->get_queue(render::QueueType::GRAPHICS);
    // auto command_list = device->create_command_list(render::QueueType::GRAPHICS);
    // auto vertex_buffer = device->create_vertex_buffer();
    
    // Vertex buffer for triangle
    // Index buffer for triangle
    // Descriptor set
    // Create and load shader

    while (window->is_open()) {
        window->process_messages();

        // Begin command list
        // Bind everything
        // Draw
        // End command list
        // Submit command list to queue
    }

    return 0;
}


