#include <iostream>
#include <print>
#include <thread>

#include "core/win/win_app_window.h"
#include "math/math.h"
#include "render/font.h"
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

struct GuiVertex {
    float pos[2];
    float uv[2];
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
    render::CommandQueue queue = {};
    render::CommandQueue copy_queue = {};
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

    create_rhi(rhi);
    rhi.enumerate_adapters(adapters, 1);
    rhi.create_device(adapters[0], device);
    device.create_queue(render::QueueType::GRAPHICS, queue);
    device.create_queue(render::QueueType::TRANSFER, copy_queue);
    device.create_command_list(render::QueueType::GRAPHICS, command_list);
    device.create_command_list(render::QueueType::TRANSFER, copy_list);

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

    rhi.create_swap_chain(queue, width, height, render::Format::R8G8B8A8_UNORM, window.hwnd(), swap_chain);
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

    // GUI setup
    render::Buffer gv_buffer = {};
    render::Buffer gc_buffer = {};
    render::Pso gui_pso = {};
    render::PsoDesc gui_pso_desc = {};
    render::BindingSet gui_binding_set = {};
    render::BindingSetDesc gui_binding_set_desc = {};
    render::Shader gui_vshader = {};
    render::Shader gui_pshader = {};

    GuiVertex rect_verts[] = {
        { 10.0f, 10.0f, 0.0f, 0.0f },
        { 200.0f, 10.0f, 0.0f, 0.0f },
        { 10.0f, 120.0f, 0.0f, 0.0f },
        { 200.0f, 10.0f, 0.0f, 0.0f },
        { 200.0f, 120.0f, 0.0f, 0.0f },
        { 10.0f, 120.0f, 0.0f, 0.0f }
    };
    device.create_buffer(1024, gv_buffer, render::HeapType::UPLOAD);
    render::upload_buffer_data(gv_buffer, 0, rect_verts, sizeof(rect_verts));
    device.create_buffer(1024, gc_buffer, render::HeapType::UPLOAD);
    float screen_size[2] = { static_cast<float>(width), static_cast<float>(height) };
    render::upload_buffer_data(gc_buffer, 0, &screen_size, sizeof(float[2]));

    render::compile_shader(L"gui_shaders.hlsl", render::ShaderType::VERTEX, "vs_main", &gui_vshader);
    render::compile_shader(L"gui_shaders.hlsl", render::ShaderType::PIXEL, "ps_main", &gui_pshader);

    gui_binding_set_desc.binding_count = 3;
    gui_binding_set_desc.bindings[0].type = render::BindingType::BUFFER_DESCRIPTOR;
    gui_binding_set_desc.bindings[1].type = render::BindingType::DESCRIPTOR_SET;
    gui_binding_set_desc.bindings[1].descriptor_set_binding.type = render::DescriptorSetRangeType::SRV;
    gui_binding_set_desc.bindings[1].descriptor_set_binding.descriptor_count = 1;
    gui_binding_set_desc.bindings[2].type = render::BindingType::DESCRIPTOR_SET;
    gui_binding_set_desc.bindings[2].descriptor_set_binding.type = render::DescriptorSetRangeType::SAMPLER;
    gui_binding_set_desc.bindings[2].descriptor_set_binding.descriptor_count = 1;
    device.create_binding_set(gui_binding_set_desc, gui_binding_set);

    gui_pso_desc.input_layout.element_count = 2;
    gui_pso_desc.input_layout.elements[0] = { "POSITION", 0, 0, 0, 0, render::Format::R32G32_FLOAT };
    gui_pso_desc.input_layout.elements[1] = { "TEXCOORD", 0, 0, 8, 0, render::Format::R32G32_FLOAT };
    gui_pso_desc.vertex_shader = &gui_vshader;
    gui_pso_desc.pixel_shader = &gui_pshader;
    gui_pso_desc.rt_count = 1;
    gui_pso_desc.rt_formats[0] = render::Format::R8G8B8A8_UNORM;
    gui_pso_desc.rasterizer.clip_depth = false;
    device.create_pso(gui_binding_set, gui_pso_desc, gui_pso);

    // Text setup
    auto tv_buffer_size = 2048;
    auto test_string = "hello";
    render::Pso text_pso = {};
    render::PsoDesc text_pso_desc = {};
    render::Shader text_vshader = {};
    render::Shader text_pshader = {};
    render::Buffer tv_buffer = {};
    render::Texture atlas_texture = {};
    render::Buffer upload_buffer = {};
    constexpr auto upload_buffer_size = 32678 * 32678;
    render::Descriptor atlas_descriptor = {};
    render::Descriptor sampler_descriptor = {};

    render::compile_shader(L"text_shaders.hlsl", render::ShaderType::VERTEX, "vs_main", &text_vshader);
    render::compile_shader(L"text_shaders.hlsl", render::ShaderType::PIXEL, "ps_main", &text_pshader);

    text_pso_desc.input_layout.element_count = 2;
    text_pso_desc.input_layout.elements[0] = { "POSITION", 0, 0, 0, 0, render::Format::R32G32_FLOAT };
    text_pso_desc.input_layout.elements[1] = { "TEXCOORD", 0, 0, 8, 0, render::Format::R32G32_FLOAT };
    text_pso_desc.vertex_shader = &text_vshader;
    text_pso_desc.pixel_shader = &text_pshader;
    text_pso_desc.rt_count = 1;
    text_pso_desc.rt_formats[0] = render::Format::R8G8B8A8_UNORM;
    text_pso_desc.rasterizer.clip_depth = false;
    text_pso_desc.blend.rts[0].blend_enable = true;
    text_pso_desc.blend.rts[0].source_blend = render::Blend::SOURCE_ALPHA;
    text_pso_desc.blend.rts[0].dest_blend = render::Blend::INV_SOURCE_ALPHA;
    device.create_pso(gui_binding_set, text_pso_desc, text_pso);

    auto atlas_info = render::generate_glyph_atlas(0x30, 0x7e, "Roboto-Regular.ttf", 36);
    device.create_texture(atlas_info.width, atlas_info.height, render::Format::R8_UNORM, atlas_texture);
    device.create_buffer(upload_buffer_size, upload_buffer, render::HeapType::UPLOAD);
    std::byte* mapped_upload_buffer;
    upload_buffer.map(reinterpret_cast<void**>(&mapped_upload_buffer));
    render::memcpy_texture_to_buffer(
        mapped_upload_buffer,
        0,
        atlas_info.bitmap,
        0,
        0,
        atlas_info.width,
        atlas_info.height,
        atlas_info.width);
    upload_buffer.unmap();

    device.create_buffer(tv_buffer_size, tv_buffer, render::HeapType::UPLOAD);
    std::byte* mapped_tv_buffer = nullptr;
    tv_buffer.map(reinterpret_cast<void**>(&mapped_tv_buffer));
    generate_text_quads(atlas_info, test_string, 10, 10, mapped_tv_buffer, tv_buffer_size);
    tv_buffer.unmap();
    atlas_descriptor = resource_descriptor_heap.allocate();
    sampler_descriptor = sampler_descriptor_heap.allocate();

    device.create_srv_descriptor(atlas_texture.d3d12_resource, nullptr, atlas_descriptor);
    device.create_sampler(render::Filter::MIN_MAG_MIP_LINEAR, sampler_descriptor);

    render::DescriptorHeap* heaps[] = { &resource_descriptor_heap, &sampler_descriptor_heap };

    copy_list.reset();
    copy_list.resource_barrier(
            atlas_texture.d3d12_resource,
            render::ResourceState::COMMON,
            render::ResourceState::COPY_DEST);
    copy_list.copy_texture(atlas_texture, 0, 0, upload_buffer);
    copy_list.resource_barrier(
        atlas_texture.d3d12_resource,
        render::ResourceState::COPY_DEST,
        render::ResourceState::COMMON);
    copy_list.close();
    copy_queue.execute(copy_list);
    copy_queue.signal(fence, 1337);
    fence.set_completion_value(1337);
    fence.wait();

    while (window.is_open()) {
        window.process_messages();

        uint32_t rt = frame_index % 2;
        ID3D12Resource* rt_buffer = nullptr;
        swap_chain.d3d12_swap_chain->GetBuffer(rt, IID_PPV_ARGS(&rt_buffer));
        constexpr float clear_color[] = { 0.0f, 0.2f, 0.4f, 1.0f };

        static auto rotate_20 = Matrix4::rotation_y(deg_to_rad(2));
        rotation = rotate_20 * rotation;
        render::upload_buffer_data(c_buffer, 2 * sizeof(Matrix4), &rotation, sizeof(rotation));

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

        // GUI stuff
        command_list.set_binding_set(gui_binding_set);
        command_list.set_pso(gui_pso);
        command_list.set_descriptor_heaps(2, heaps);
        command_list.set_descriptor_set(1, atlas_descriptor);
        command_list.set_descriptor_set(2, sampler_descriptor);
        command_list.set_constant_buffer(0, gc_buffer);
        command_list.set_vertex_buffer(gv_buffer, sizeof(GuiVertex));
        command_list.draw(6, 0);

        command_list.set_pso(text_pso);
        command_list.set_descriptor_heaps(2, heaps);
        command_list.set_vertex_buffer(tv_buffer, 4 * sizeof(float));
        command_list.draw(strlen(test_string) * 6, 0);
        // /GUI stuff

        command_list.resource_barrier(rt_buffer, render::ResourceState::RENDER_TARGET, render::ResourceState::PRESENT);
        command_list.close();
        queue.execute(command_list);
        swap_chain.present();
        queue.signal(fence, frame_index);
        fence.set_completion_value(frame_index);
        fence.wait();
        frame_index++;

        rt_buffer->Release();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
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
