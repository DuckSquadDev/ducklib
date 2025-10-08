#ifndef DUCKLIB_GUI_H
#define DUCKLIB_GUI_H
#include <cstdint>
#include <span>

#include "font.h"
#include "input/input.h"
#include "math/math.h"
#include "render/rhi/device.h"
#include "render/rhi/types.h"

namespace ducklib::gui {
constexpr uint32_t GUI_INVALID_ID = -1;

struct GuiVertex {
    float x, y;
    float u, v;
};

struct GuiState {
    static constexpr auto SHAPE_BUFFER_SIZE = 1024;
    static constexpr auto TEXT_BUFFER_SIZE = 1048576;

    uint32_t focused_id = GUI_INVALID_ID;
    uint32_t id_counter = 0;
    render::Buffer shape_vbuffer;
    render::Buffer text_vbuffer;
    GuiVertex shape_staging_vbuffer[SHAPE_BUFFER_SIZE];
    GuiVertex text_staging_vbuffer[TEXT_BUFFER_SIZE];
    uint32_t shape_staged_vertex_count = 0;
    uint32_t text_staged_vertex_count = 0;

    uint32_t window_width;
    uint32_t window_height;

    render::BindingSet binding_set = {};
    render::Pso gui_pso = {};
    render::Shader gui_vshader = {};
    render::Shader gui_pshader = {};
    render::Pso text_pso = {};
    render::Shader text_vshader = {};
    render::Shader text_pshader = {};
    render::GlyphAtlasInfo glyph_atlas;
    render::Texture glyph_atlas_texture = {};
    render::Buffer gui_cbuffer = {};
    render::Descriptor glyph_atlas_descriptor = {};
    render::Descriptor glyph_sampler_descriptor = {};

    InputState* input_state = {};
};

void draw_rect(GuiState& gui_state, Rect rect);
void draw_label(GuiState& gui_state, Rect rect, std::u8string_view text);
void draw_edit(GuiState& gui_state, Rect rect, std::span<char8_t> text_buffer, uint32_t& text_bytes);

void init_gui_state(
    GuiState& gui_state,
    render::Device& device,
    uint32_t window_width,
    uint32_t window_height,
    render::DescriptorHeap srv_heap,
    render::DescriptorHeap sampler_heap);
void reset_gui_state(GuiState& gui_state);
void draw_gui_state(GuiState& gui_state, render::CommandList cmd_list, render::DescriptorHeap** heaps);
}

#endif //DUCKLIB_GUI_H
