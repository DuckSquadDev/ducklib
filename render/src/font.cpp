#include <array>
#include <fstream>
#include <assert.h>

#define STB_TRUETYPE_IMPLEMENTATION

#include "../lib/stb_truetype.h"
#include "render/font.h"

namespace ducklib::render {
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

    for (auto i = 0; i < glyph_height; ++i) {
        memcpy(&atlas_bitmap[write_caret], &glyph_bitmap[read_caret], glyph_width);
        write_caret += atlas_width;
        read_caret += glyph_stride;
    }
}

GlyphAtlasInfo generate_glyph_atlas(int codepoint_start, int codepoint_end, std::string_view font, uint8_t size) {
    // Read font file
    auto font_file = fopen(font.data(), "rb");
    fseek(font_file, 0, SEEK_END);
    auto font_file_size = ftell(font_file);
    rewind(font_file);
    auto font_buffer = new std::byte[font_file_size];
    fread(font_buffer, 1, font_file_size, font_file);
    fclose(font_file);

    // Load read font file into stbtt
    stbtt_fontinfo font_info = {};
    stbtt_InitFont(&font_info, reinterpret_cast<const unsigned char*>(font_buffer), 0);
    auto scale = stbtt_ScaleForPixelHeight(&font_info, size);
    constexpr uint32_t atlas_width = 1024;

    // First pass: gather glyph info (it's also used to decide atlas size)
    constexpr auto glyph_padding = 2;
    auto max_glyph_height = 0;
    auto rows_required = 1;
    auto caret_pos = glyph_padding;
    std::map<uint32_t, GlyphInfo> glyph_infos;
    int ascent;
    int descent;
    int line_gap;

    stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &line_gap);

    for (auto i = codepoint_start; i <= codepoint_end; ++i) {
        int x0, y0;
        int x1, y1;
        int advance_width;
        int left_bearing;

        stbtt_GetCodepointHMetrics(&font_info, i, &advance_width, &left_bearing);
        stbtt_GetCodepointBitmapBox(&font_info, i, scale, scale, &x0, &y0, &x1, &y1);
        assert(y1 - y0 >= 0);
        assert(x1 - x0 >= 0);
        uint32_t height = y1 - y0;
        uint32_t width = x1 - x0;

        if (height > max_glyph_height) {
            max_glyph_height = height;
        }

        if (caret_pos + 2 * glyph_padding + width > atlas_width) {
            rows_required++;
            caret_pos = width + 2 * glyph_padding;
        }

        GlyphInfo glyph_entry = {
            0.0f, // temp UV coords
            0.0f,
            0.0f,
            0.0f,
            static_cast<int>(left_bearing * scale),
            static_cast<int>(y0 * scale),
            width,
            height,
            static_cast<int>(advance_width * scale)
        };
        glyph_infos.insert({ i, glyph_entry });
    }

    // Second pass: create and render glyphs onto atlas
    constexpr auto glyph_bitmap_width = 128;
    constexpr auto glyph_bitmap_height = 128;
    auto glyph_bitmap = new std::byte[glyph_bitmap_width * glyph_bitmap_height * 100];
    uint32_t atlas_height = rows_required * (max_glyph_height + glyph_padding) + glyph_padding;
    uint32_t atlas_size = atlas_width * atlas_height;
    auto atlas_bitmap = new std::byte[atlas_size];
    auto caret_x = glyph_padding;
    auto caret_y = glyph_padding;

    memset(atlas_bitmap, 0, atlas_size);

    for (auto i = codepoint_start; i <= codepoint_end; ++i) {
        auto& glyph_info = glyph_infos[i];

        stbtt_MakeCodepointBitmap(
            &font_info,
            reinterpret_cast<unsigned char*>(glyph_bitmap),
            glyph_bitmap_width,
            glyph_bitmap_height,
            glyph_bitmap_width,
            scale,
            scale,
            i);

        if (caret_x + glyph_info.width > atlas_width + glyph_padding) {
            caret_x = glyph_padding;
            caret_y += max_glyph_height + glyph_padding;
        }

        assert(caret_y <= atlas_height - glyph_padding - max_glyph_height);

        copy_glyph_into_atlas(
            glyph_bitmap,
            glyph_info.width,
            glyph_info.height,
            glyph_bitmap_width,
            atlas_bitmap,
            atlas_width,
            atlas_height,
            caret_x,
            caret_y);

        glyph_info.u0 = static_cast<float>(caret_x) / atlas_width;
        glyph_info.v0 = static_cast<float>(caret_y) / atlas_height;
        glyph_info.u1 = (static_cast<float>(caret_x) + glyph_info.width) / atlas_width;
        glyph_info.v1 = (static_cast<float>(caret_y) + glyph_info.height) / atlas_height;
        caret_x += glyph_info.width + glyph_padding;
    }

    delete[] glyph_bitmap;
    delete[] font_buffer;

    return {
        atlas_bitmap,
        atlas_width,
        atlas_height,
        static_cast<int>(ascent * scale),
        static_cast<int>(descent * scale),
        line_gap,
        std::move(glyph_infos)
    };
}

void generate_text_quads(
    const GlyphAtlasInfo& atlas,
    std::string_view text,
    int x,
    int y,
    std::byte* vertex_buffer,
    uint32_t vertex_buffer_size) {
    
    struct GlyphVertex {
        float x, y;
        float u, v;
    };
    
    constexpr auto glyph_stride = 6 * sizeof(GlyphVertex);
    auto caret_x = x;
    auto caret_y = y + atlas.ascent; // TODO: Fix y-offset (y is top-left of element, probably, which is bad here)
    auto vertices = reinterpret_cast<GlyphVertex*>(vertex_buffer);

    if (text.length() * glyph_stride > vertex_buffer_size) {
        // TODO: Handle error better
        throw std::runtime_error("Vertex buffer size for text is too small");
    }

    for (auto i = 0; i < text.length(); ++i) {
        auto& glyph_info = atlas.glyph_infos.at(static_cast<uint32_t>(text[i]));
        auto x0 = static_cast<float>(caret_x + glyph_info.x_offset);
        auto x1 = static_cast<float>(caret_x + glyph_info.x_offset + glyph_info.width);
        auto y0 = static_cast<float>(caret_y - glyph_info.height);
        auto y1 = static_cast<float>(caret_y);

        vertices[i * 6 + 0] = GlyphVertex{ x0, y0, glyph_info.u0, glyph_info.v0 };
        vertices[i * 6 + 1] = GlyphVertex{ x1, y0, glyph_info.u1, glyph_info.v0 };
        vertices[i * 6 + 2] = GlyphVertex{ x0, y1, glyph_info.u0, glyph_info.v1 };
        vertices[i * 6 + 3] = GlyphVertex{ x0, y1, glyph_info.u0, glyph_info.v1 };
        vertices[i * 6 + 4] = GlyphVertex{ x1, y0, glyph_info.u1, glyph_info.v0 };
        vertices[i * 6 + 5] = GlyphVertex{ x1, y1, glyph_info.u1, glyph_info.v1 };

        caret_x += glyph_info.x_advance;
    }
}
}
