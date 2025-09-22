// #include <cassert>
// #include <Windows.h>
#include <memory>
#include <print>
#include <thread>

#include "core/win/win_app_window.h"
#include "math/math.h"
#include "render/render_util.h"
#include "render/resource_manager.h"
#include "render/rhi/rhi.h"
#include "render/rhi/types.h"
#include "render/rhi/shader.h"

using namespace ducklib;

constexpr int width = 640;
constexpr int height = 360;

struct Vertex {
    float pos[4];
    float color[4];
};

void output(std::string_view message, LogLevel level, std::source_location source_location) {
    std::println("{}", message.data());
}

int __stdcall WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* cmdLine, int cmdShow) {
    WinAppWindow window{ "Hello world!", width, height };
    render::Rhi rhi = {};
    render::SwapChain swap_chain = {};
    render::Adapter adapters[1] = {};
    render::Device device = {};
    render::CommandQueue queue = {};
    render::CommandList command_list = {};
    render::Buffer v_buffer = {};
    render::Buffer c_buffer = {};
    render::Descriptor cb_descriptor = {};
    render::DescriptorHeap rt_descriptor_heap = {};
    render::DescriptorHeap resource_descriptor_heap = {};
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

    // render::log = output;

    create_rhi(rhi);
    rhi.enumerate_adapters(adapters, 1);
    rhi.create_device(adapters[0], device);
    device.create_queue(render::QueueType::GRAPHICS, queue);
    device.create_command_list(render::QueueType::GRAPHICS, command_list);

    compile_shader(L"shaders.hlsl", render::ShaderType::VERTEX, "vs_main", &vertex_shader);
    compile_shader(L"shaders.hlsl", render::ShaderType::PIXEL, "ps_main", &pixel_shader);

    binding_set_desc.binding_count = 1;
    binding_set_desc.bindings[0].type = render::BindingType::BUFFER_DESCRIPTOR;
    binding_set_desc.bindings[0].descriptor_binding.register_space = 0;
    binding_set_desc.bindings[0].descriptor_binding.shader_register = 0;

    device.create_binding_set(binding_set_desc, binding_set);
    pso_desc.input_layout.element_count = 2;
    pso_desc.input_layout.elements[0] = { "POSITION", 0, 0, 0, 0, render::Format::R32G32B32A32_FLOAT };
    pso_desc.input_layout.elements[1] = { "COLOR", 0, 0, 16, 0, render::Format::R32G32B32A32_FLOAT };
    pso_desc.vertex_shader = &vertex_shader;
    pso_desc.pixel_shader = &pixel_shader;
    pso_desc.rt_count = 1;
    pso_desc.rt_formats[0] = render::Format::R8G8B8A8_UNORM;
    pso_desc.rasterizer.cull_mode = render::CullMode::NONE;
    device.create_pso(binding_set, pso_desc, pso);

    Vertex vertices[] = {
        { { 0.0f, 0.375f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { 0.25f, -0.375f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.25f, -0.375f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };
    device.create_buffer(sizeof(vertices), v_buffer, render::HeapType::UPLOAD);
    render::upload_buffer_data(&v_buffer, 0, vertices, sizeof(vertices));

    device.create_descriptor_heap(render::DescriptorHeapType::RT, 128, rt_descriptor_heap);
    device.create_descriptor_heap(render::DescriptorHeapType::CBV_SRV_UAV, 128, resource_descriptor_heap);
    device.create_fence(frame_index, fence);

    rhi.create_swap_chain(queue, width, height, render::Format::R8G8B8A8_UNORM, window.hwnd(), swap_chain);
    rt_descriptors[0] = { .cpu_handle = rt_descriptor_heap.cpu_handle(0) };
    swap_chain.d3d12_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    device.create_rt_descriptor(back_buffer, nullptr, rt_descriptors[0]);
    back_buffer->Release();
    rt_descriptors[1] = { .cpu_handle = rt_descriptor_heap.cpu_handle(1) };
    swap_chain.d3d12_swap_chain->GetBuffer(1, IID_PPV_ARGS(&back_buffer));
    device.create_rt_descriptor(back_buffer, nullptr, rt_descriptors[1]);
    back_buffer->Release();

    auto view = Matrix4::look_at_lh(
        { 0.0f, 0.0f, -2.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f });
    auto projection = Matrix4::perspective_lh(deg_to_rad(50), 16.0f / 9.0f, 0.0001f, 10000.0f);
    auto rotation = Matrix4::identity;
    device.create_buffer(256, c_buffer, render::HeapType::UPLOAD);
    render::upload_buffer_data(&c_buffer, 0, &view, sizeof(view));
    render::upload_buffer_data(&c_buffer, sizeof(Matrix4), &projection, sizeof(projection));
    cb_descriptor = { .cpu_handle = resource_descriptor_heap.cpu_handle(0) };
    device.create_cbuffer_descriptor(c_buffer, cb_descriptor);

    while (window.is_open()) {
        window.process_messages();

        uint32_t rt = frame_index % 2;
        ID3D12Resource* rt_buffer = nullptr;
        swap_chain.d3d12_swap_chain->GetBuffer(rt, IID_PPV_ARGS(&rt_buffer));
        constexpr float clear_color[] = { 0.0f, 0.2f, 0.4f, 1.0f };

        static auto rotate_20 = Matrix4::rotation_y(deg_to_rad(2));
        rotation = rotate_20 * rotation;
        render::upload_buffer_data(&c_buffer, 2 * sizeof(Matrix4), &rotation, sizeof(rotation));

        command_list.reset(&pso);
        command_list.set_binding_set(binding_set);
        command_list.set_viewport(0.0f, 0.0f, width, height);
        command_list.set_scissor_rect(0.0f, 0.0f, width, height);
        command_list.resource_barrier(rt_buffer, render::ResourceState::PRESENT, render::ResourceState::RENDER_TARGET);
        command_list.set_rt(rt_descriptors[rt]);
        command_list.clear_rt(rt_descriptors[rt], clear_color);
        command_list.set_primitive_topology(render::PrimitiveTopology::TRIANGLE);
        command_list.set_constant_buffer(0, c_buffer);
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

        rt_buffer->Release();

        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }

    c_buffer.d3d12_resource->Release();
    fence.d3d12_fence->Release();
    swap_chain.d3d12_swap_chain->Release();
    resource_descriptor_heap.d3d12_heap->Release();
    rt_descriptor_heap.d3d12_heap->Release();
    v_buffer.d3d12_resource->Release();
    pso.d3d12_pso->Release();
    binding_set.d3d12_signature->Release();
    pixel_shader.d3d_bytecode_blob->Release();
    vertex_shader.d3d_bytecode_blob->Release();

    return 0;
}
