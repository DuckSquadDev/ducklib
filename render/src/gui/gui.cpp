#include "render/gui/gui.h"
#include "render/gui/font.h"

namespace ducklib::gui {
auto default_font = render::generate_glyph_atlas(0x30, 0x7e, "Roboto-Regular.ttf", 24);

void submit_rect_vertices(GuiState& gui_state, Rect rect);

void draw_rect(GuiState& gui_state, Rect rect) {
    submit_rect_vertices(gui_state, rect);
}

void draw_label(GuiState& gui_state, Rect rect, std::string_view text) {
    submit_rect_vertices(gui_state, rect);

    auto remaining_text_vertices = GuiState::TEXT_BUFFER_SIZE - gui_state.text_staged_count;
    auto quads_rendered = render::generate_text_quads(
        default_font,
        text,
        rect.x,
        rect.y,
        &gui_state.text_staging_buffer[gui_state.text_staged_count],
        remaining_text_vertices);
    gui_state.shape_staged_count += quads_rendered * 6;
}

void draw_edit(GuiState& gui_state, Rect rect, std::span<char> text_buffer) {
    auto id = gui_state.id_counter++;

    if (gui_state.focused_id == id) {
        // TODO: Consume inputs
    }

    submit_rect_vertices(gui_state, rect);

    auto remaining_text_vertices = GuiState::TEXT_BUFFER_SIZE - gui_state.text_staged_count;
    auto quads_rendered = render::generate_text_quads(
        default_font,
        std::string_view{ text_buffer },
        rect.x,
        rect.y,
        &gui_state.text_staging_buffer[gui_state.text_staged_count],
        remaining_text_vertices);
    gui_state.shape_staged_count += quads_rendered * 6;

    // TODO: Check mouse click for focus
}

void submit_rect_vertices(GuiState& gui_state, Rect rect) {
    auto i = gui_state.shape_staged_count;
    auto x0 = static_cast<float>(rect.x);
    auto x1 = static_cast<float>(rect.x + rect.width);
    auto y0 = static_cast<float>(rect.y);
    auto y1 = static_cast<float>(rect.y + rect.height);

    gui_state.shape_staging_buffer[i + 0] = { x0, y0, 0.0f, 0.0f };
    gui_state.shape_staging_buffer[i + 1] = { x1, y0, 0.0f, 0.0f };
    gui_state.shape_staging_buffer[i + 2] = { x0, y1, 0.0f, 0.0f };
    gui_state.shape_staging_buffer[i + 3] = gui_state.shape_staging_buffer[i + 2];
    gui_state.shape_staging_buffer[i + 4] = gui_state.shape_staging_buffer[i + 1];
    gui_state.shape_staging_buffer[i + 5] = { x1, y1, 0.0f, 0.0f };

    gui_state.shape_staged_count += 6;
}
}
