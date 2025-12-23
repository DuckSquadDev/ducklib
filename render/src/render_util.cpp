#include "ducklib/render/render_util.h"

namespace ducklib::render {
void memcpy_texture_to_buffer(
    std::byte* dest,
    uint32_t dest_offset,
    const std::byte* source,
    uint32_t source_x,
    uint32_t source_y,
    uint32_t source_width,
    uint32_t source_height,
    uint32_t pitch) {
    auto dest_caret = dest_offset;
    auto source_caret = source_y * pitch + source_x;

    for (auto i = 0; i < source_height; ++i) {
        memcpy(&dest[dest_caret], &source[source_caret], source_width);
        dest_caret += source_width;
        source_caret += pitch;
    }
}
}
