#include "core/unicode.h"

#include <cstring>

namespace ducklib {
char32_t utf16_to_cp(const char16_t* str, uint32_t word_len) {
    if (0xd800 <= str[0] && str[0] <= 0xdbff) {
        if (word_len < 2) {
            return UNICODE_INVALID;
        }
        return 0x10000 + ((str[0] & 0x3ff) << 10) | (str[1] & 0x3ff);
    }
    return static_cast<char32_t>(str[0]);
}

char32_t utf8_to_cp(const char8_t* str, uint32_t byte_len) {
    constexpr uint8_t len_table[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 3, 4, 0
    };
    constexpr char8_t start_mask[] = {
        0x00, 0x7f, 0x1f, 0x0f, 0x07
    };
    constexpr uint8_t shift[] = { 0, 0, 0, 0, 6, 12, 18 }; // Extra zeroes to avoid negative shifts on the last step

    char8_t s[4] = {};
    auto len = len_table[str[0] >> 3];
    auto ok_len = len > byte_len ? 0 : len;

    if (ok_len == 0) {
        return UNICODE_INVALID;
    }
    
    memcpy(s, str, len);
    
    auto cp0 = s[0] & start_mask[len];
    auto cp1 = s[1] & 0x3f;
    auto cp2 = s[2] & 0x3f;
    auto cp3 = s[3] & 0x3f;
    return cp0 << shift[len + 3 - 1] // + 3 here to ensure safe shifts
        | cp1 << shift[len + 3 - 2]
        | cp2 << shift[len + 3 - 3]
        | cp3 << shift[len + 3 - 4];
}
}
