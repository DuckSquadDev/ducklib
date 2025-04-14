// #include <cassert>
// #include <Windows.h>
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
    render::DescriptorHeap rt_descriptor_heap = {};
    render::BindingSetDesc binding_set_desc = {};
    render::BindingSet binding_set = {};
    render::Shader vertex_shader = {};
    render::Shader pixel_shader = {};
    render::PsoDesc pso_desc = {};
    render::Pso pso = {};
    render::Fence fence = {};
    render::Descriptor rt_descriptors[2] = {};
    uint32_t frame_index = 0;
    ID3D12Resource* back_buffer = nullptr;

    create_rhi(rhi);
    rhi.enumerate_adapters(adapters, 1);
    rhi.create_device(adapters[0], device);
    device.create_queue(render::QueueType::GRAPHICS, queue);
    device.create_command_list(render::QueueType::GRAPHICS, command_list);

    compile_shader(L"../render/test/render_hello_world/d3d_shaders.hlsl", render::ShaderType::VERTEX, "VSMain", &vertex_shader);
    compile_shader(L"../render/test/render_hello_world/d3d_shaders.hlsl", render::ShaderType::PIXEL, "PSMain", &pixel_shader);

    device.create_binding_set(binding_set_desc, binding_set);
    pso_desc.input_layout.element_count = 2;
    pso_desc.input_layout.elements[0] = { "POSITION", 0, 0, 0, 0, render::Format::R32G32B32_FLOAT };
    pso_desc.input_layout.elements[1] = { "COLOR", 0, 0, 12, 0, render::Format::R32G32B32A32_FLOAT };
    pso_desc.vertex_shader = &vertex_shader;
    pso_desc.pixel_shader = &pixel_shader;
    pso_desc.rt_count = 1;
    pso_desc.rt_formats[0] = render::Format::R8G8B8A8_UNORM;
    device.create_pso(binding_set, pso_desc, pso);

    Vertex vertices[] = {
        { { 0.0f, 0.375f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { 0.25f, -0.375f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.25f, -0.375f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };
    device.create_buffer(sizeof(vertices), v_buffer, render::HeapType::UPLOAD);
    upload_buffer_data(&v_buffer, 0, vertices, sizeof(vertices));

    device.create_descriptor_heap(render::DescriptorHeapType::RT, 128, rt_descriptor_heap);
    device.create_fence(frame_index, fence);
    
    rhi.create_swap_chain(queue, 600, 400, render::Format::R8G8B8A8_UNORM, window.hwnd(), swap_chain);
    rt_descriptors[0] = { .cpu_handle = rt_descriptor_heap.cpu_handle(0) };
    swap_chain.d3d12_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    device.create_rt_descriptor(back_buffer, nullptr, rt_descriptors[0]);
    rt_descriptors[1] = { .cpu_handle = rt_descriptor_heap.cpu_handle(1) };
    swap_chain.d3d12_swap_chain->GetBuffer(1, IID_PPV_ARGS(&back_buffer));
    device.create_rt_descriptor(back_buffer, nullptr, rt_descriptors[1]);

    while (window.is_open()) {
        window.process_messages();

        uint32_t rt = frame_index % 2;
        ID3D12Resource* rt_buffer = nullptr;
        swap_chain.d3d12_swap_chain->GetBuffer(rt, IID_PPV_ARGS(&rt_buffer));
        const float clear_color[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        
        command_list.reset(&pso);
        command_list.set_binding_set(binding_set);
        command_list.set_viewport(0.0f, 0.0f, 600.0f, 400.0f);
        command_list.set_scissor_rect(0.0f, 0.0f, 600.0f, 400.0f);
        command_list.resource_barrier(rt_buffer, render::ResourceState::PRESENT, render::ResourceState::RENDER_TARGET);
        command_list.set_rt(rt_descriptors[rt]);
        command_list.clear_rt(rt_descriptors[rt], clear_color);
        command_list.set_primitive_topology(render::PrimitiveTopology::TRIANGLE);
        command_list.set_vertex_buffer(v_buffer, sizeof(Vertex));
        command_list.draw(3, 0);
        command_list.resource_barrier(rt_buffer, render::ResourceState::RENDER_TARGET, render::ResourceState::PRESENT);
        command_list.close();
        queue.execute(command_list);
        swap_chain.present();
        queue.signal(fence, frame_index);
        fence.set_completion_value(frame_index);
        fence.wait();
        frame_index++;

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}
