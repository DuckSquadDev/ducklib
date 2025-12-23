#include <algorithm>
#include <array>
#include <fstream>
#include <cassert>

#define STB_TRUETYPE_IMPLEMENTATION

#include "third_party/stb_truetype.h"
#include "ducklib/render/gui/font.h"
#include "ducklib/render/gui/gui.h"

#undef max

namespace ducklib::render {
constexpr char32_t FONT_MISSING_CODEPOINT = 0xfffc;
constexpr auto FONT_MISSING_INDEX = 0;
constexpr auto GLYPH_PADDING = 2U;

// @details Assumes 1 byte texel size.
void copy_glyph_into_atlas(
    const std::byte* glyph_bitmap,
    uint32_t glyph_width,
    uint32_t glyph_height,
    uint32_t glyph_stride,
    std::byte* atlas_bitmap,
    uint32_t atlas_width,
    uint32_t atlas_height,
    uint32_t dest_x,
    uint32_t dest_y) {
    assert(atlas_height >= dest_y + glyph_height);
    assert(atlas_width >= dest_x + glyph_width);

    auto write_caret = dest_y * atlas_width + dest_x;
    auto read_caret = 0;

    for (auto i = 0U; i < glyph_height; ++i) {
        memcpy(&atlas_bitmap[write_caret], &glyph_bitmap[read_caret], glyph_width);
        write_caret += atlas_width;
        read_caret += glyph_stride;
    }
}

GlyphInfo append_glyph_info(
    const stbtt_fontinfo& font_info,
    char32_t codepoint,
    float scale,
    uint32_t& x_caret,
    uint32_t& rows_required,
    uint32_t& max_glyph_height,
    uint32_t atlas_width) {
    int x0, y0;
    int x1, y1;
    int advance_width;
    int left_bearing;

    stbtt_GetCodepointHMetrics(&font_info, codepoint, &advance_width, &left_bearing);
    stbtt_GetCodepointBitmapBox(&font_info, codepoint, scale, scale, &x0, &y0, &x1, &y1);
    assert(y1 - y0 >= 0);
    assert(x1 - x0 >= 0);
    uint32_t height = y1 - y0;
    uint32_t width = x1 - x0;
    max_glyph_height = std::max(max_glyph_height, height);

    if (x_caret + width + 2 * GLYPH_PADDING > atlas_width) {
        rows_required++;
        x_caret = width + 2 * GLYPH_PADDING;
    } else {
        x_caret += width + GLYPH_PADDING;
    }

    return {
        0.0f, // temp UV coords
        0.0f,
        0.0f,
        0.0f,
        static_cast<int>(left_bearing * scale),
        y0,
        width,
        height,
        static_cast<int>(advance_width * scale)
    };
}

uint32_t get_glyph_width(const stbtt_fontinfo& font_info, float scale, char32_t codepoint) {
    int advance_width;
    int left_offset;

    stbtt_GetCodepointHMetrics(&font_info, static_cast<int>(codepoint), &advance_width, &left_offset);

    return static_cast<uint32_t>(std::round(static_cast<float>(advance_width) * scale));
}

void draw_glyph_and_advance(
    const stbtt_fontinfo& font_info,
    char32_t codepoint,
    float scale,
    std::byte* atlas_bitmap,
    uint32_t atlas_width,
    uint32_t atlas_height,
    uint32_t max_glyph_height,
    GlyphInfo& glyph_info,
    uint32_t& x_caret,
    uint32_t& y_caret) {
    constexpr auto GLYPH_BITMAP_WIDTH = 128;
    constexpr auto GLYPH_BITMAP_HEIGHT = 128;
    static std::byte glyph_bitmap[GLYPH_BITMAP_WIDTH * GLYPH_BITMAP_HEIGHT * 100];

    stbtt_MakeCodepointBitmap(
        &font_info,
        reinterpret_cast<unsigned char*>(glyph_bitmap),
        GLYPH_BITMAP_WIDTH,
        GLYPH_BITMAP_HEIGHT,
        GLYPH_BITMAP_WIDTH,
        scale,
        scale,
        codepoint);

    if (x_caret + glyph_info.width + GLYPH_PADDING > atlas_width) {
        x_caret = GLYPH_PADDING;
        y_caret += max_glyph_height + GLYPH_PADDING;
    }

    assert(y_caret <= atlas_height - GLYPH_PADDING - max_glyph_height);

    copy_glyph_into_atlas(
        glyph_bitmap,
        glyph_info.width,
        glyph_info.height,
        GLYPH_BITMAP_WIDTH,
        atlas_bitmap,
        atlas_width,
        atlas_height,
        x_caret,
        y_caret);

    glyph_info.u0 = x_caret / (float)atlas_width;
    glyph_info.v0 = y_caret / (float)atlas_height;
    glyph_info.u1 = (x_caret + (float)glyph_info.width) / atlas_width;
    glyph_info.v1 = (y_caret + (float)glyph_info.height) / atlas_height;
    x_caret += glyph_info.width + GLYPH_PADDING;
}

#define CHECK_FILE(op, msg) \
    do { \
        if ((op) != 0) { \
            throw std::runtime_error(msg); \
        } \
    } while(false)

GlyphAtlasInfo generate_glyph_atlas(std::span<GlyphRange> codepoint_ranges, std::u8string_view font, uint8_t size) {
    // Read the font file
    FILE* font_file = nullptr;
    auto result = fopen_s(&font_file, reinterpret_cast<const char*>(font.data()), "rb");
    if (result == 0) {
        throw std::runtime_error("Failed to open font file");
    }
    CHECK_FILE(fseek(font_file, 0, SEEK_END), "Failed to seek in font file");
    auto font_file_size = ftell(font_file);
    CHECK_FILE(fseek(font_file, 0, SEEK_SET), "Failed to seek in font file");
    auto font_buffer = new std::byte[font_file_size];
    CHECK_FILE(fread(font_buffer, 1, font_file_size, font_file), "Failed to read font file");
    CHECK_FILE(fclose(font_file), "Failed to close font file");

    // Load read font file into stbtt
    stbtt_fontinfo font_info = {};
    stbtt_InitFont(&font_info, reinterpret_cast<const unsigned char*>(font_buffer), 0);
    auto scale = stbtt_ScaleForPixelHeight(&font_info, size);
    constexpr uint32_t atlas_width = 1024;

    // First pass: gather glyph info (it's also used to decide atlas size)
    auto max_glyph_height = 0U;
    auto rows_required = 1U;
    auto caret_pos = GLYPH_PADDING;
    std::map<uint32_t, GlyphInfo> glyph_infos;
    int ascent;
    int descent;
    int line_gap;

    stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &line_gap);
    auto space_width = get_glyph_width(font_info, scale, ' ');

    glyph_infos.insert(
        {
            FONT_MISSING_INDEX,
            append_glyph_info(font_info, FONT_MISSING_CODEPOINT, scale, caret_pos, rows_required, max_glyph_height, atlas_width)
        });

    for (auto range : codepoint_ranges) {
        for (auto i = range.start; i <= range.end; ++i) {
            glyph_infos.insert({ i, append_glyph_info(font_info, i, scale, caret_pos, rows_required, max_glyph_height, atlas_width) });
        }
    }

    // Second pass: create and render glyphs onto atlas
    uint32_t atlas_height = rows_required * (max_glyph_height + GLYPH_PADDING) + GLYPH_PADDING;
    uint32_t atlas_size = atlas_width * atlas_height;
    auto atlas_bitmap = new std::byte[atlas_size];
    auto x_caret = GLYPH_PADDING;
    auto y_caret = GLYPH_PADDING;

    memset(atlas_bitmap, 0, atlas_size);

    draw_glyph_and_advance(
        font_info,
        FONT_MISSING_CODEPOINT,
        scale,
        atlas_bitmap,
        atlas_width,
        atlas_height,
        max_glyph_height,
        glyph_infos[FONT_MISSING_INDEX],
        x_caret,
        y_caret);

    for (auto range : codepoint_ranges) {
        for (auto i = range.start; i <= range.end; ++i) {
            draw_glyph_and_advance(
                font_info,
                i,
                scale,
                atlas_bitmap,
                atlas_width,
                atlas_height,
                max_glyph_height,
                glyph_infos[i],
                x_caret,
                y_caret);
        }
    }

    delete[] font_buffer;

    return {
        atlas_bitmap,
        atlas_width,
        atlas_height,
        static_cast<int>(ascent * scale),
        static_cast<int>(descent * scale),
        line_gap,
        std::move(glyph_infos),
        space_width
    };
}

uint32_t generate_text_quads(
    const GlyphAtlasInfo& atlas,
    std::u8string_view text,
    int x,
    int y,
    gui::GuiVertex* vertex_buffer,
    uint32_t vertex_buffer_size) {

    constexpr auto glyph_stride = 6 * sizeof(gui::GuiVertex);
    auto caret_x = x;
    auto caret_y = y + atlas.ascent;
    auto g = 0; // glyphs written

    if (text.length() * glyph_stride > vertex_buffer_size) {
        // TODO: Handle error better
        throw std::runtime_error("Vertex buffer size for text is too small");
    }

    for (auto i = 0U; i < text.length(); ++i) {
        auto ch = static_cast<uint32_t>(text[i]);

        if (text[i] == ' ') {
            caret_x += atlas.space_width;
            continue;
        }
        if (!atlas.glyph_infos.contains(static_cast<uint32_t>(ch))) {
            ch = FONT_MISSING_INDEX;
        }

        auto& glyph_info = atlas.glyph_infos.at(static_cast<uint32_t>(ch));
        auto x0 = static_cast<float>(caret_x + glyph_info.x_offset);
        auto x1 = static_cast<float>(caret_x + glyph_info.x_offset + glyph_info.width);
        auto y0 = static_cast<float>(caret_y + glyph_info.y_offset);
        auto y1 = static_cast<float>(caret_y + glyph_info.y_offset + glyph_info.height);

        float color[] = { 0.4f, 0.7f, 0.0f, 1.0f };
        vertex_buffer[g * 6 + 0] = gui::GuiVertex { x0, y0, glyph_info.u0, glyph_info.v0, { color[0], color[1], color[2], color[3] } };
        vertex_buffer[g * 6 + 1] = gui::GuiVertex { x1, y0, glyph_info.u1, glyph_info.v0, { color[0], color[1], color[2], color[3] } };
        vertex_buffer[g * 6 + 2] = gui::GuiVertex { x0, y1, glyph_info.u0, glyph_info.v1, { color[0], color[1], color[2], color[3] } };
        vertex_buffer[g * 6 + 3] = gui::GuiVertex { x0, y1, glyph_info.u0, glyph_info.v1, { color[0], color[1], color[2], color[3] } };
        vertex_buffer[g * 6 + 4] = gui::GuiVertex { x1, y0, glyph_info.u1, glyph_info.v0, { color[0], color[1], color[2], color[3] } };
        vertex_buffer[g * 6 + 5] = gui::GuiVertex { x1, y1, glyph_info.u1, glyph_info.v1, { color[0], color[1], color[2], color[3] } };

        caret_x += glyph_info.x_advance;
        ++g;
    }

    return g;
}
}
