#include <cassert>
#include <Windows.h>
#include <memory>

#include "core/win/win_app_window.h"
#include "render/resource_manager.h"
#include "render/rhi/rhi.h"

using namespace ducklib;

int __stdcall WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* cmdLine, int cmdShow) {
    WinAppWindow window{ "Hello world!", 600, 400 };
    render::Rhi rhi;
    render::Adapter adapters[1];
    render::Device device;
    render::CommandQueue queue;
    render::CommandList command_list;
    render::Buffer v_buffer;
    render::Buffer c_buffer;
    render::DescriptorHeap descriptor_heap;

    create_rhi(&rhi);
    rhi.enumerate_adapters(adapters, 1);
    rhi.create_device(&adapters[0], &device);
    device.create_queue(render::QueueType::GRAPHICS, &queue);
    device.create_command_list(render::QueueType::GRAPHICS, &command_list);

    device.create_buffer(256, &v_buffer, render::HeapType::UPLOAD);
    float triangle_points[3][3] = { { -1.0f, 0.0f, 0.0f }, { 0.0f, 2.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } };
    upload_buffer_data(&v_buffer, 0, triangle_points, sizeof(triangle_points));

    device.create_buffer(256, &c_buffer, render::HeapType::UPLOAD);
    float view_matrix[4][4] = {
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, -5.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    };
    float perspective_matrix[4][4] = {
        { 2.4142f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 2.4142f, 0.0f, 0.0f },
        { 0.0f, 0.0f, -1.002f, -0.2002f },
        { 0.0f, 0.0f, -1.0f, 0.0f }
    };
    upload_buffer_data(&c_buffer, 0, view_matrix, sizeof(view_matrix));
    upload_buffer_data(&c_buffer, sizeof(view_matrix), perspective_matrix, sizeof(view_matrix));
    
    while (window.is_open()) {
        window.process_messages();
    }

    return 0;
}
