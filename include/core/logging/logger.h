#pragma once
#include <cstdint>
#include <format>
#include <mutex>
#include <regex>
#include <span>

#include "log_level.h"

#undef ERROR

namespace ducklib {
class Logger {
public:
    auto set_log_level(LogLevel new_log_level) -> void;

    template <typename... Args>
    auto log(LogLevel level, std::string_view text, const Args&... args) -> void;
    auto log(LogLevel level, std::string_view text) -> void;

    template <typename... Args>
    auto log_error(std::string_view text, const Args&... args) -> void;
    auto log_error(std::string_view text) -> void;
    template <typename... Args>
    auto log_warn(std::string_view text, const Args&... args) -> void;
    auto log_warn(std::string_view text) -> void;
    template <typename... Args>
    auto log_info(std::string_view text, const Args&... args) -> void;
    auto log_info(std::string_view text) -> void;
    template <typename... Args>
    auto log_debug(std::string_view text, const Args&... args) -> void;
    auto log_debug(std::string_view text) -> void;

private:
    static constexpr uint16_t MessageBufferSize = 512;
    static constexpr uint16_t MessageFormatBufferSize = MessageBufferSize;

    static std::mutex stdout_mutex;

    /**
     * \return Prefix length in chars
     */
    static auto write_prefix_to_message(const std::span<char>& buffer, LogLevel level) -> std::span<char>::size_type;
    static auto stdout_sink(std::string_view text) -> void; // TODO: Move to somewhere external

    LogLevel log_level_setting = LogLevel::WARNING;
};

inline auto Logger::set_log_level(LogLevel new_log_level) -> void {
    log_level_setting = new_log_level;
}

template <typename... Args>
auto Logger::log_error(std::string_view text, const Args&... args) -> void {
    return Log(LogLevel::ERROR, text, args...);
}

template <typename... Args>
auto Logger::log_warn(std::string_view text, const Args&... args) -> void {
    return Log(LogLevel::WARNING, text, args...);
}

template <typename... Args>
auto Logger::log_info(std::string_view text, const Args&... args) -> void {
    return Log(LogLevel::INFO, text, args...);
}

template <typename... Args>
auto Logger::log_debug(std::string_view text, const Args&... args) -> void {
    return Log(LogLevel::DEBUG, text, args...);
}

template <typename... Args>
auto Logger::log(LogLevel level, std::string_view text, const Args&... args) -> void {
    if (level > log_level_setting)
        return;

    auto format_buffer = std::array<char, MessageFormatBufferSize>{};
    auto end_it = std::format_to(format_buffer.begin(), text, args...);
    auto formatted_message = std::string_view{ format_buffer.begin(), std::distance(format_buffer.begin(), end_it) };

    return log(level, formatted_message);
}
}
