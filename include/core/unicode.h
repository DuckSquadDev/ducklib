#ifndef DUCKLIB_UNICODE_H
#define DUCKLIB_UNICODE_H
#include <cstdint>

namespace ducklib {
constexpr char32_t UNICODE_INVALID = 0xfffd;

char32_t utf16_to_cp(const char16_t* str, uint32_t word_len);
char32_t utf8_to_cp(const char8_t* str, uint32_t byte_len);
/// @return Number of code units written
uint8_t cp_to_utf8(char32_t cp, char8_t* str, uint32_t buffer_len);
}

#endif //DUCKLIB_UNICODE_H