#ifndef DUCKLIB_UNICODE_H
#define DUCKLIB_UNICODE_H
#include <cstdint>

namespace ducklib {
constexpr char32_t UNICODE_INVALID = 0xfffd;

char32_t utf16_to_cp(const char16_t* str, uint32_t word_len);
char32_t utf8_to_cp(const char8_t* str, uint32_t byte_len);
uint8_t cp_to_utf8(char32_t utf32_ch, char8_t buffer[4]);
}

#endif //DUCKLIB_UNICODE_H