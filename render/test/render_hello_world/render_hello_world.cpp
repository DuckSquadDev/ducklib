#include <cassert>
#include <Windows.h>
#include <memory>
#include <thread>

#include "core/win/win_app_window.h"
#include "render/resource_manager.h"
#include "render/rhi/rhi.h"
#include "render/rhi/types.h"
#include "render/rhi/shader.h"

using namespace ducklib;

struct Vertex {
    float pos[3];
    float color[4];
};

int __stdcall WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* cmdLine, int cmdShow) {
    WinAppWindow window{ "Hello world!", 600, 400 };
    render::Rhi rhi = {};
    render::SwapChain swap_chain = {};
    render::Adapter adapters[1] = {};
    render::Device device = {};
    render::CommandQueue queue = {};
    render::CommandList command_list = {};
    render::Buffer v_buffer = {};
    render::Buffer c_buffer = {};
    render::DescriptorHeap def_descriptor_heap = {};
    render::DescriptorHeap rt_descriptor_heap = {};
    render::BindingSetDesc binding_set_desc = {};
    render::BindingSet binding_set = {};
    render::Shader vertex_shader = {};
    render::Shader pixel_shader = {};
    render::PsoDesc pso_desc = {};
    render::Pso pso = {};
    render::Fence fence = {};
    render::Descriptor cb_descriptor = {};
    render::Descriptor rt_descriptors[2] = {};
    uint32_t frame_index = 0;
    ID3D12Resource* backBuffer = nullptr;

    create_rhi(rhi);
    rhi.enumerate_adapters(adapters, 1);
    rhi.create_device(adapters[0], device);
    device.create_queue(render::QueueType::GRAPHICS, queue);
    device.create_command_list(render::QueueType::GRAPHICS, command_list);
    device.create_descriptor_heap(render::DescriptorHeapType::CBV_SRV_UAV, 128, def_descriptor_heap);
    device.create_descriptor_heap(render::DescriptorHeapType::RT, 128, rt_descriptor_heap);
    device.create_fence(frame_index, fence);

    rhi.create_swap_chain(queue, 600, 400, render::Format::R8G8B8A8_UNORM, window.hwnd(), swap_chain);
    rt_descriptors[0] = { .cpu_handle = rt_descriptor_heap.cpu_handle(0) };
    swap_chain.d3d12_swap_chain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    device.create_rt_descriptor(backBuffer, nullptr, rt_descriptors[0]);
    rt_descriptors[1] = { .cpu_handle = rt_descriptor_heap.cpu_handle(1) };
    swap_chain.d3d12_swap_chain->GetBuffer(1, IID_PPV_ARGS(&backBuffer));
    device.create_rt_descriptor(backBuffer, nullptr, rt_descriptors[1]);

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
    cb_descriptor = { .cpu_handle = def_descriptor_heap.cpu_handle(0), .gpu_handle = def_descriptor_heap.gpu_handle(0) };
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

        command_list.reset();

        uint32_t rt = frame_index % 2;
        constexpr float clear_color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
        ID3D12Resource* rt_buffer = nullptr;
        swap_chain.d3d12_swap_chain->GetBuffer(rt, IID_PPV_ARGS(&rt_buffer));

        command_list.resource_barrier(rt_buffer, render::ResourceState::PRESENT, render::ResourceState::RENDER_TARGET);
        command_list.set_rt(rt_descriptors[rt]);
        command_list.clear_rt(rt_descriptors[rt], clear_color);
        command_list.resource_barrier(rt_buffer, render::ResourceState::RENDER_TARGET, render::ResourceState::PRESENT);
        
        command_list.close();
        queue.execute(command_list);

        swap_chain.present();
        queue.signal(fence, frame_index);
        fence.set_completion_value(frame_index);
        frame_index++;
        fence.wait();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}
