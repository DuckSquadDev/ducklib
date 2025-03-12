#include <cassert>
#include <Windows.h>
#include <memory>

#include "core/win/win_app_window.h"
#include "render/resource_manager.h"
#include "render/rhi/rhi.h"
#include "render/rhi/types.h"
#include "render/rhi/shader.h"

using namespace ducklib;

int __stdcall WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* cmdLine, int cmdShow) {
    WinAppWindow window{ "Hello world!", 600, 400 };
    render::Rhi rhi = {};
    render::Adapter adapters[1] = {};
    render::Device device = {};
    render::CommandQueue queue = {};
    render::CommandList command_list = {};
    render::Buffer v_buffer = {};
    render::Buffer c_buffer = {};
    render::DescriptorHeap descriptor_heap = {};
    render::BindingSetDesc binding_set_desc = {};
    render::BindingSet binding_set = {};
    render::Shader vertex_shader = {};
    render::Shader pixel_shader = {};
    render::PsoDesc pso_desc = {};
    render::Pso pso = {};
    render::Descriptor cb_descriptor = {};

    create_rhi(&rhi);
    rhi.enumerate_adapters(adapters, 1);
    rhi.create_device(&adapters[0], &device);
    device.create_queue(render::QueueType::GRAPHICS, queue);
    device.create_command_list(render::QueueType::GRAPHICS, command_list);
    device.create_descriptor_heap(render::DescriptorHeapType::CBV_SRV_UAV, 128, descriptor_heap);

    device.create_buffer(256, v_buffer, render::HeapType::UPLOAD);
    float triangle_points[3][3] = { { -1.0f, 0.0f, 0.0f }, { 0.0f, 2.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } };
    upload_buffer_data(&v_buffer, 0, triangle_points, sizeof(triangle_points));

    device.create_buffer(256, c_buffer, render::HeapType::UPLOAD);
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
    cb_descriptor = { .cpu_handle = descriptor_heap.cpu_handle(0), .gpu_handle = descriptor_heap.gpu_handle(0) };
    device.create_cbuffer_descriptor(c_buffer, cb_descriptor);

    binding_set_desc.binding_count = 1;
    binding_set_desc.bindings[0].type = render::BindingType::BUFFER_DESCRIPTOR;
    binding_set_desc.bindings[0].descriptor_binding.register_space = 0;
    binding_set_desc.bindings[0].descriptor_binding.shader_register = 0;
    device.create_binding_set(binding_set_desc, binding_set);

    compile_shader(L"../render/test/render_hello_world/shaders.hlsl", render::ShaderType::VERTEX, "vs_main", &vertex_shader);
    compile_shader(L"../render/test/render_hello_world/shaders.hlsl", render::ShaderType::PIXEL, "ps_main", &pixel_shader);

    pso_desc.input_layout.element_count = 1;
    pso_desc.input_layout.elements[0].format = render::Format::R32G32B32_FLOAT;
    pso_desc.input_layout.elements[0].semantic_name = "POSITION";
    pso_desc.vertex_shader = &vertex_shader;
    pso_desc.pixel_shader = &pixel_shader;
    pso_desc.rt_count = 1;
    pso_desc.rt_formats[0] = render::Format::R8G8B8A8_UINT;

    device.create_pso(binding_set, pso_desc, pso);

    while (window.is_open()) {
        window.process_messages();
    }

    return 0;
}
