#include <cmath>
#include <cstring>

#include "core/unicode.h"

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

uint8_t cp_to_utf8(char32_t cp, char8_t* str, uint32_t buffer_len) {
    if (cp < 0x80) {
        str[0] = static_cast<char8_t>(cp);
        return 1;
    } else if (cp < 0x800) {
        if (buffer_len < 2) {
            return 0;
        }
        str[0] = 0xc0 | (cp >> 6) & 0x7f;
        str[1] = 0x80 | cp & 0x3f;
        return 2;
    } else if (cp < 0x10000) {
        if (buffer_len < 2) {
            return 0;
        }
        str[0] = 0xe0 | (cp >> 12) & 0x1f;
        str[1] = 0x80 | (cp >> 6) & 0x3f;
        str[2] = 0x80 | cp & 0x3f;
        return 3;
    } else {
        if (buffer_len < 2) {
            return 0;
        }
        str[0] = 0xf0 | (cp >> 18) & 0x0f;
        str[1] = 0x80 | (cp >> 12) & 0x1f;
        str[2] = 0x80 | (cp >> 6) & 0x3f;
        str[3] = 0x80 | cp & 0x3f;
        return 4;
    }
}

/// @brief Returns the number of bytes to move back to get to the start of the current UTF-8 code point
/// @param last_byte Pointer to the last byte/code unit of a UTF-8 code point
/// @param buffer_byte_count The number of bytes in the buffer before the `last_byte` pointer
int8_t prev_utf8(const char8_t* last_byte, uint32_t buffer_byte_count) {
    auto bytes_left = buffer_byte_count;
    auto i = 0;
    
    while (bytes_left > 0 && (last_byte[i] & 0xc0) == 0x80) {
        --bytes_left;
        --i;
    }

    return std::abs(i);
}

/// @brief Returns the number of code units of the current UTF-8 code point. Assumes ptr starts on the first code unit.
uint8_t utf8_ch_size(const char8_t* first_byte, uint32_t bytes_left) {
    auto i = 1;
    
    while(bytes_left > 0 && (first_byte[i] & 0xc0) == 0x80) {
        ++i;
        --bytes_left;
    }

    return i;
}
}
