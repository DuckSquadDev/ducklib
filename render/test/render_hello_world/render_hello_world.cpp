#include <Windows.h>
#include <memory>

#include "core/win/win_app_window.h"
#include "render/rhi/rhi.h"

using namespace ducklib;

int __stdcall WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* cmdLine, int cmdShow) {
    WinAppWindow window{ "Hello world!", 600, 400 };
    render::Rhi rhi;
    render::Adapter adapters[1];
    render::Device device;
    render::CommandQueue queue;
    render::VertexBuffer v_buffer;

    create_rhi(&rhi);
    rhi.enumerate_adapters(adapters, 1);
    rhi.create_device(&adapters[0], &device);
    device.create_queue(render::QueueType::GRAPHICS, &queue);
    device.create_vertex_buffer(256, &v_buffer);
    
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

    while (window.is_open()) {
        window.process_messages();

        // Begin command list
        // Bind everything
        // Draw
        // End command list
        // Submit command list to queue
    }

    return 0;
}
