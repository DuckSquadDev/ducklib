#ifndef DUCKLIB_GUI_H
#define DUCKLIB_GUI_H
#include <cstdint>
#include <span>

#include "math/math.h"
#include "render/rhi/types.h"

namespace ducklib::gui {
constexpr uint32_t GUI_INVALID_ID = -1;

struct GuiVertex {
    float x, y;
    float u, v;
};

struct GuiState {
    static constexpr auto SHAPE_BUFFER_SIZE = 1024;
    static constexpr auto TEXT_BUFFER_SIZE = 2048;
    
    uint32_t focused_id = GUI_INVALID_ID;
    uint32_t id_counter = 0;
    render::Buffer shape_vbuffer;
    render::Buffer text_vbuffer;
    GuiVertex shape_staging_buffer[SHAPE_BUFFER_SIZE];
    GuiVertex text_staging_buffer[TEXT_BUFFER_SIZE];
    uint32_t shape_staged_count = 0;
    uint32_t text_staged_count = 0;
};

void draw_rect(GuiState& gui_state, Rect rect);
void draw_label(GuiState& gui_state, Rect rect, std::string_view text);
void draw_edit(GuiState& gui_state, Rect rect, std::span<char> text_buffer);

void reset_gui_state(GuiState& gui_state);
void draw_gui_state(GuiState& gui_state);
}

#endif //DUCKLIB_GUI_H