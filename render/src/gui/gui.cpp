#include <iostream>

#include "render/gui/gui.h"
#include "core/unicode.h"
#include "render/resource_manager.h"
#include "render/gui/font.h"
#include "render/rhi/shader.h"

namespace ducklib::gui {
auto default_font = render::generate_glyph_atlas(0x30, 0x7e, u8"Roboto-Regular.ttf", 24);

void submit_rect_vertices(GuiState& gui_state, Rect rect, const float color[] = DEF_RECT_COLOR);

void draw_rect(GuiState& gui_state, Rect rect) {
    submit_rect_vertices(gui_state, rect);
}

void draw_label(GuiState& gui_state, Rect rect, std::u8string_view text) {
    submit_rect_vertices(gui_state, rect);

    auto remaining_text_vertices = GuiState::TEXT_BUFFER_SIZE - gui_state.text_staged_vertex_count;
    auto quads_rendered = render::generate_text_quads(
        default_font,
        text,
        rect.x,
        rect.y,
        &gui_state.text_staging_vbuffer[gui_state.text_staged_vertex_count],
        remaining_text_vertices);
    gui_state.shape_staged_vertex_count += quads_rendered * 6;
}

bool pos_in_rect(int x, int y, Rect rect) {
    if (rect.x <= x && x <= rect.x + rect.width
        && rect.y <= y && y <= rect.y + rect.height) {
         return true;
    }

    return false;
}

void modify_text_from_input(std::span<char8_t> text_buffer, uint32_t& text_bytes, std::span<char8_t> inputs) {
    auto text_input_bytes = inputs.size();

    for (auto i = 0; i < text_input_bytes;) {
        switch (inputs[i]) {
        case 0x8: {
            // Backspace
            if (text_bytes == 0) {
                ++i;
                continue;
            }
            auto move_back = prev_utf8(&text_buffer[text_bytes - 1], text_bytes - 1);
            text_bytes -= move_back + 1; // + 1 as the move_back doesn't include the step back we did above when finding the start of the code point
            ++i;
            break;
        }
        default: {
            auto code_units = utf8_ch_size(&inputs[i], text_input_bytes - (i + 1));
            memcpy(&text_buffer[text_bytes], &inputs[i], code_units);
            text_bytes += code_units;
            i += code_units;
            break;
        }
        }
    }
}

bool check_hover(const GuiState& gui_state, Rect rect) {
    if (gui_state.input_state) {
        const auto input = gui_state.input_state;
        return pos_in_rect(input->mouse_x, input->mouse_y, rect);
    }

    return false;
}

bool update_focus(GuiState& gui_state, Rect rect, uint32_t control_id) {
    if (gui_state.input_state) {
        const auto* input = gui_state.input_state;
        if (pos_in_rect(input->mouse_x, input->mouse_y, rect) && input->mouse_released(0)) {
            gui_state.focused_id = control_id;
            gui_state.frame_clicked_id = control_id;
            return true;
        }
    }

    return false;
}

void draw_edit(GuiState& gui_state, Rect rect, std::span<char8_t> text_buffer, uint32_t& text_bytes) {
    auto id = gui_state.id_counter++;
    auto color = DEF_RECT_COLOR;

    if (id == gui_state.focused_id) {
        if (gui_state.input_state) {
            modify_text_from_input(text_buffer, text_bytes, gui_state.input_state->text_inputs);
            gui_state.input_state->text_inputs.clear();
        }
        color = DEF_RECT_COLOR_HL2;
    } else if (check_hover(gui_state, rect)) {
        color = DEF_RECT_COLOR_HL;
    }
    
    update_focus(gui_state, rect, id);
    submit_rect_vertices(gui_state, rect, color);

    auto remaining_text_vertices = GuiState::TEXT_BUFFER_SIZE - gui_state.text_staged_vertex_count;
    auto quads_rendered = render::generate_text_quads(
        default_font,
        std::u8string_view{ text_buffer.data(), text_bytes },
        rect.x,
        rect.y,
        &gui_state.text_staging_vbuffer[gui_state.text_staged_vertex_count],
        remaining_text_vertices);
    gui_state.text_staged_vertex_count += quads_rendered * 6;
}

void create_glyph_atlas_texture(
    GuiState& gui_state,
    render::Device& device,
    render::DescriptorHeap srv_heap,
    render::DescriptorHeap sampler_heap) {

    constexpr auto upload_buffer_size = 4096 * 4096;
    render::Buffer upload_buffer = {};
    auto atlas_info = render::generate_glyph_atlas(0x30, 0x7e, u8"Roboto-Regular.ttf", 24);
    device.create_texture(atlas_info.width, atlas_info.height, render::Format::R8_UNORM, gui_state.glyph_atlas_texture);
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
    gui_state.glyph_atlas = atlas_info;
    gui_state.glyph_atlas_descriptor = srv_heap.allocate();
    gui_state.glyph_sampler_descriptor = sampler_heap.allocate();
    device.create_srv_descriptor(gui_state.glyph_atlas_texture.d3d12_resource, nullptr, gui_state.glyph_atlas_descriptor);
    device.create_sampler(render::Filter::MIN_MAG_MIP_LINEAR, gui_state.glyph_sampler_descriptor);

    render::CommandList copy_list = {};
    render::Fence fence = {};
    device.create_command_list(render::QueueType::COPY, copy_list);
    device.create_fence(0, fence);

    copy_list.reset();
    copy_list.resource_barrier(
        gui_state.glyph_atlas_texture.d3d12_resource,
        render::ResourceState::COMMON,
        render::ResourceState::COPY_DEST);
    copy_list.copy_texture(gui_state.glyph_atlas_texture, 0, 0, upload_buffer);
    copy_list.resource_barrier(
        gui_state.glyph_atlas_texture.d3d12_resource,
        render::ResourceState::COPY_DEST,
        render::ResourceState::COMMON);
    copy_list.close();
    device.copy_queue.execute(copy_list);
    device.copy_queue.signal(fence, 1337);
    fence.set_completion_value(1337);
    fence.wait();
}

void init_gui_state(
    GuiState& gui_state,
    render::Device& device,
    uint32_t window_width,
    uint32_t window_height,
    render::DescriptorHeap srv_heap,
    render::DescriptorHeap sampler_heap) {
    
    render::BindingSetDesc binding_set_desc = {};
    render::PsoDesc gui_pso_desc = {};
    render::PsoDesc text_pso_desc = {};

    render::compile_shader(L"gui_shaders.hlsl", render::ShaderType::VERTEX, "vs_main", &gui_state.gui_vshader);
    render::compile_shader(L"gui_shaders.hlsl", render::ShaderType::PIXEL, "ps_main", &gui_state.gui_pshader);
    render::compile_shader(L"text_shaders.hlsl", render::ShaderType::VERTEX, "vs_main", &gui_state.text_vshader);
    render::compile_shader(L"text_shaders.hlsl", render::ShaderType::PIXEL, "ps_main", &gui_state.text_pshader);

    binding_set_desc.binding_count = 3;
    binding_set_desc.bindings[0].type = render::BindingType::BUFFER_DESCRIPTOR;
    binding_set_desc.bindings[1].type = render::BindingType::DESCRIPTOR_SET;
    binding_set_desc.bindings[1].descriptor_set_binding.type = render::DescriptorSetRangeType::SRV;
    binding_set_desc.bindings[1].descriptor_set_binding.descriptor_count = 1;
    binding_set_desc.bindings[2].type = render::BindingType::DESCRIPTOR_SET;
    binding_set_desc.bindings[2].descriptor_set_binding.type = render::DescriptorSetRangeType::SAMPLER;
    binding_set_desc.bindings[2].descriptor_set_binding.descriptor_count = 1;
    device.create_binding_set(binding_set_desc, gui_state.binding_set);

    gui_pso_desc.input_layout.element_count = 3;
    gui_pso_desc.input_layout.elements[0] = { "POSITION", 0, 0, 0, 0, render::Format::R32G32_FLOAT };
    gui_pso_desc.input_layout.elements[1] = { "TEXCOORD", 0, 0, 8, 0, render::Format::R32G32_FLOAT };
    gui_pso_desc.input_layout.elements[2] = { "COLOR", 0, 0, 16, 0, render::Format::R32G32B32A32_FLOAT };
    gui_pso_desc.vertex_shader = &gui_state.gui_vshader;
    gui_pso_desc.pixel_shader = &gui_state.gui_pshader;
    gui_pso_desc.rt_count = 1;
    gui_pso_desc.rt_formats[0] = render::Format::R8G8B8A8_UNORM;
    gui_pso_desc.rasterizer.clip_depth = false;
    device.create_pso(gui_state.binding_set, gui_pso_desc, gui_state.gui_pso);

    text_pso_desc.input_layout.element_count = 3;
    text_pso_desc.input_layout.elements[0] = { "POSITION", 0, 0, 0, 0, render::Format::R32G32_FLOAT };
    text_pso_desc.input_layout.elements[1] = { "TEXCOORD", 0, 0, 8, 0, render::Format::R32G32_FLOAT };
    text_pso_desc.input_layout.elements[2] = { "COLOR", 0, 0, 16, 0, render::Format::R32G32B32A32_FLOAT };
    text_pso_desc.vertex_shader = &gui_state.text_vshader;
    text_pso_desc.pixel_shader = &gui_state.text_pshader;
    text_pso_desc.rt_count = 1;
    text_pso_desc.rt_formats[0] = render::Format::R8G8B8A8_UNORM;
    text_pso_desc.rasterizer.clip_depth = false;
    text_pso_desc.blend.rts[0].blend_enable = true;
    text_pso_desc.blend.rts[0].source_blend = render::Blend::SOURCE_ALPHA;
    text_pso_desc.blend.rts[0].dest_blend = render::Blend::INV_SOURCE_ALPHA;
    device.create_pso(gui_state.binding_set, text_pso_desc, gui_state.text_pso);

    device.create_buffer(1024, gui_state.gui_cbuffer, render::HeapType::UPLOAD);
    float screen_size[2] = { static_cast<float>(window_width), static_cast<float>(window_height) };
    render::upload_buffer_data(gui_state.gui_cbuffer, 0, &screen_size, sizeof(screen_size));

    device.create_buffer(GuiState::SHAPE_BUFFER_SIZE, gui_state.shape_vbuffer, render::HeapType::UPLOAD);
    device.create_buffer(GuiState::TEXT_BUFFER_SIZE, gui_state.text_vbuffer, render::HeapType::UPLOAD);

    create_glyph_atlas_texture(gui_state, device, srv_heap, sampler_heap);
}

void submit_rect_vertices(GuiState& gui_state, Rect rect, const float color[]) {
    auto i = gui_state.shape_staged_vertex_count;
    auto x0 = static_cast<float>(rect.x);
    auto x1 = static_cast<float>(rect.x + rect.width);
    auto y0 = static_cast<float>(rect.y);
    auto y1 = static_cast<float>(rect.y + rect.height);

    gui_state.shape_staging_vbuffer[i + 0] = { x0, y0, 0.0f, 0.0f, color[0], color[1], color[2], color[3] };
    gui_state.shape_staging_vbuffer[i + 1] = { x1, y0, 0.0f, 0.0f, color[0], color[1], color[2], color[3] };
    gui_state.shape_staging_vbuffer[i + 2] = { x0, y1, 0.0f, 0.0f, color[0], color[1], color[2], color[3] };
    gui_state.shape_staging_vbuffer[i + 3] = gui_state.shape_staging_vbuffer[i + 2];
    gui_state.shape_staging_vbuffer[i + 4] = gui_state.shape_staging_vbuffer[i + 1];
    gui_state.shape_staging_vbuffer[i + 5] = { x1, y1, 0.0f, 0.0f, color[0], color[1], color[2], color[3] };

    gui_state.shape_staged_vertex_count += 6;
}

void frame_reset_gui_state(GuiState& gui_state) {
    gui_state.frame_clicked_id = GUI_INVALID_ID;
    gui_state.id_counter = 0;
    gui_state.shape_staged_vertex_count = 0;
    gui_state.text_staged_vertex_count = 0;
}

void draw_gui_state(GuiState& gui_state, render::CommandList cmd_list, render::DescriptorHeap** heaps) {
    // Upload rects and texts
    GuiVertex* vertex_ptr;
    gui_state.shape_vbuffer.map(reinterpret_cast<void**>(&vertex_ptr));
    memcpy(vertex_ptr, gui_state.shape_staging_vbuffer, sizeof(GuiVertex) * gui_state.shape_staged_vertex_count);
    gui_state.shape_vbuffer.unmap();
    gui_state.text_vbuffer.map(reinterpret_cast<void**>(&vertex_ptr));
    memcpy(vertex_ptr, gui_state.text_staging_vbuffer, sizeof(GuiVertex) * gui_state.text_staged_vertex_count);
    gui_state.text_vbuffer.unmap();

    // Draw it all
    cmd_list.set_binding_set(gui_state.binding_set);
    cmd_list.set_pso(gui_state.gui_pso);
    cmd_list.set_descriptor_heaps(2, heaps);
    cmd_list.set_descriptor_set(1, gui_state.glyph_atlas_descriptor);
    cmd_list.set_descriptor_set(2, gui_state.glyph_sampler_descriptor);
    cmd_list.set_constant_buffer(0, gui_state.gui_cbuffer);
    cmd_list.set_vertex_buffer(gui_state.shape_vbuffer, sizeof(GuiVertex));
    cmd_list.draw(gui_state.shape_staged_vertex_count, 0);

    cmd_list.set_pso(gui_state.text_pso);
    cmd_list.set_descriptor_heaps(2, heaps);
    cmd_list.set_vertex_buffer(gui_state.text_vbuffer, sizeof(GuiVertex));
    cmd_list.draw(gui_state.text_staged_vertex_count, 0);
}

void check_controls_defocused(GuiState& gui_state) {
    if (gui_state.input_state && gui_state.input_state->mouse_released(0) && gui_state.frame_clicked_id == GUI_INVALID_ID) {
        gui_state.focused_id = GUI_INVALID_ID;
    }
}
}
