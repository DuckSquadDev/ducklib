#include <array>
#include <iostream>
#include <print>
#include <thread>

#include "ducklib/core/win/win_app_window.h"
#include "ducklib/core/math.h"
#include "ducklib/render/gui/font.h"
#include "ducklib/core/unicode.h"
#include "ducklib/input/input.h"
#include "ducklib/render/render_util.h"
#include "ducklib/render/resource_manager.h"
#include "ducklib/render/gui/gui.h"
#include "ducklib/render/rhi/rhi.h"
#include "ducklib/render/rhi/types.h"
#include "ducklib/render/rhi/shader.h"

using namespace ducklib;

constexpr int width = 640;
constexpr int height = 360;

struct Vertex {
    float pos[4];
    float color[4];
};

void output(std::string_view message, LogLevel level, std::source_location source_location) {
    std::print("{} ({}): {}", source_location.file_name(), source_location.line(), message.data());
    std::cout.flush();
}

int __stdcall WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* cmdLine, int cmdShow) {
    WinAppWindow window{ "Hello world!", width, height };
    render::Rhi rhi = {};
    render::SwapChain swap_chain = {};
    render::Adapter adapters[1] = {};
    render::Device device = {};
    render::CommandList command_list = {};
    render::CommandList copy_list = {};
    render::Buffer v_buffer = {};
    render::Buffer c_buffer = {};
    render::Descriptor cb_descriptor = {};
    render::DescriptorHeap rt_descriptor_heap = {};
    render::DescriptorHeap resource_descriptor_heap = {};
    render::DescriptorHeap sampler_descriptor_heap = {};
    render::BindingSetDesc binding_set_desc = {};
    render::BindingSet binding_set = {};
    render::Shader vertex_shader = {};
    render::Shader pixel_shader = {};
    render::PsoDesc pso_desc = {};
    render::Pso pso = {};
    render::Fence fence = {};
    render::Fence copy_fence = {};
    render::Descriptor rt_descriptors[2] = {};
    uint32_t frame_index = 0;
    ID3D12Resource* back_buffer = nullptr;

    render::log = output;
    InputState input_state = {};
    register_raw_win_input(window.hwnd());
    window.register_message_callback(
        [&input_state](AppWindow* app_window, uint32_t msg, WPARAM wParam, LPARAM lParam) {
            process_win_input(app_window, input_state, msg, wParam, lParam);
        });

    create_rhi(rhi);
    rhi.enumerate_adapters(adapters, 1);
    rhi.create_device(adapters[0], device);
    device.create_command_list(render::QueueType::GRAPHICS, command_list);
    device.create_command_list(render::QueueType::COPY, copy_list);

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
    render::upload_buffer_data(v_buffer, 0, vertices, sizeof(vertices));

    device.create_descriptor_heap(render::DescriptorHeapType::RT, 128, rt_descriptor_heap);
    device.create_descriptor_heap(render::DescriptorHeapType::CBV_SRV_UAV, 128, resource_descriptor_heap);
    device.create_descriptor_heap(render::DescriptorHeapType::SAMPLER, 128, sampler_descriptor_heap);
    device.create_fence(frame_index, fence);
    device.create_fence(0, copy_fence);

    rhi.create_swap_chain(device, rt_descriptor_heap, 2, width, height, render::Format::R8G8B8A8_UNORM, window.hwnd(), swap_chain);
    rt_descriptors[0] = rt_descriptor_heap.allocate();
    swap_chain.d3d12_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    device.create_rt_descriptor(back_buffer, nullptr, rt_descriptors[0]);
    back_buffer->Release();
    rt_descriptors[1] = rt_descriptor_heap.allocate();
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
    render::upload_buffer_data(c_buffer, 0, &view, sizeof(view));
    render::upload_buffer_data(c_buffer, sizeof(Matrix4), &projection, sizeof(projection));
    cb_descriptor = resource_descriptor_heap.allocate();
    device.create_cbuffer_descriptor(c_buffer, cb_descriptor);

    render::DescriptorHeap* heaps[] = { &resource_descriptor_heap, &sampler_descriptor_heap };

    // New GUI stuff
    auto gui_state = std::make_unique<gui::GuiState>();
    std::array<char8_t, 2048> text_buffer;
    auto text_bytes = 2u;
    text_buffer[0] = 'h';
    text_buffer[1] = 'i';

    gui::init_gui_state(*gui_state, device, width, height, resource_descriptor_heap, sampler_descriptor_heap);
    gui_state->input_state = &input_state;

    while (window.is_open()) {
        frame_input_reset(input_state);
        window.process_messages();

        auto& rt_buffer = swap_chain.current_buffer();
        auto rt_descriptor = swap_chain.current_buffer_descriptor();
        constexpr float clear_color[] = { 0.0f, 0.2f, 0.4f, 1.0f };

        static auto rotate_20 = Matrix4::rotation_y(deg_to_rad(2));
        rotation = rotate_20 * rotation;
        render::upload_buffer_data(c_buffer, 2 * sizeof(Matrix4), &rotation, sizeof(rotation));

        // New GUI stuff
        frame_reset_gui_state(*gui_state);
        gui::draw_edit(*gui_state, { 40, 20, 200, 30 }, text_buffer, text_bytes);
        gui::draw_button(
            *gui_state,
            { 40, 80, 100, 30 },
            u8"click me",
            [] {
                std::println("clicked");
                std::cout.flush();
            });

        // Triangle rendering
        command_list.reset(&pso);
        command_list.set_binding_set(binding_set);
        command_list.set_viewport(0.0f, 0.0f, width, height);
        command_list.set_scissor_rect(0.0f, 0.0f, width, height);
        command_list.resource_barrier(rt_buffer, render::ResourceState::PRESENT, render::ResourceState::RENDER_TARGET);
        command_list.set_rt(rt_descriptor);
        command_list.clear_rt(rt_descriptor, clear_color);
        command_list.set_primitive_topology(render::PrimitiveTopology::TRIANGLE);
        command_list.set_constant_buffer(0, c_buffer);
        command_list.set_vertex_buffer(v_buffer, sizeof(Vertex));
        command_list.draw(3, 0);

        // GUI rendering
        draw_gui_state(*gui_state, command_list, heaps);

        command_list.resource_barrier(rt_buffer, render::ResourceState::RENDER_TARGET, render::ResourceState::PRESENT);
        command_list.close();
        device.graphics_queue.execute(command_list);
        swap_chain.present();
        device.graphics_queue.signal(fence, frame_index);
        fence.set_completion_value(frame_index);
        fence.wait();
        frame_index++;

        gui::check_controls_defocused(*gui_state);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    unregister_raw_win_input();

    return 0;
}
