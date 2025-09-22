#ifndef DUCKLIB_RENDER_UTIL_H
#define DUCKLIB_RENDER_UTIL_H
#include <print>
#include <source_location>
#include <string_view>
#include <Windows.h>

#include "../core/logging/log_level.h"

namespace ducklib::render {
// inline void (*log)(std::string_view message, LogLevel level, std::source_location source_location);
inline void log(std::string_view message, LogLevel level, std::source_location source_location) {
    std::println("{}", message.data());
}

inline LPSTR format_check_message(HRESULT hresult, std::string_view text) {
    LPSTR msg_ptr;
    
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        hresult,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&msg_ptr),
        0,
        nullptr);

    return msg_ptr;
}

#define DL_CHECK_D3D(expr) \
    do { \
        HRESULT result = (expr); \
        if (FAILED(result)) { \
                auto message = format_check_message(result, #expr); \
                log(message, ::ducklib::LogLevel::ERROR, std::source_location::current()); \
                LocalFree(message); \
            return; \
        } \
    } while (false)

#define DL_CHECK_D3D_RET(expr, ret_value) \
    do { \
        HRESULT result = (expr); \
        if (FAILED(result)) { \
            if (log) { \
                auto message = format_check_message(result, #expr); \
                log(message, ::ducklib::LogLevel::ERROR, std::source_location::current()); \
                LocalFree(message); \
            } \
            return ret_value; \
        } \
    } while (false)

#define DL_CHECK_D3D_THROW(expr) \
    do { \
        HRESULT result = (expr); \
        if (FAILED(result)) { \
            if (log) { \
                auto message = format_check_message(result, #expr); \
                log(message, ::ducklib::LogLevel::ERROR, std::source_location::current()); \
                LocalFree(message); \
            } \
            throw std::exception(#expr); \
        } \
    } while (false)
}

#endif //DUCKLIB_RENDER_UTIL_H
