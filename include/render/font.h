#ifndef DUCKLIB_FONT_H
#define DUCKLIB_FONT_H
#include <map>
#include <string_view>
#include <stdfloat>

namespace ducklib::render {
struct GlyphInfo {
    float u0, v0;
    float u1, v1;
    int x_offset;
    int y_offset;
    uint32_t width;
    uint32_t height;
    int x_advance;
};

struct GlyphAtlasInfo {
    const std::byte* bitmap;
    uint32_t width;
    uint32_t height;
    int ascent;
    int descent;
    int line_gap;
    std::map<uint32_t, GlyphInfo> glyph_infos;
    uint32_t space_width;
};

GlyphAtlasInfo generate_glyph_atlas(int codepoint_start, int codepoint_end, std::string_view font, uint8_t size = 16);
void generate_text_quads(
    const GlyphAtlasInfo& atlas,
    std::string_view text,
    int x,
    int y,
    std::byte* vertex_buffer,
    uint32_t vertex_buffer_size);
}

#endif //DUCKLIB_FONT_H
