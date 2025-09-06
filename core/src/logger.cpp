#include <iostream>
#include <mutex>
#include <iterator>
#include <array>

#include "../../include/core/logging/logger.h"


namespace ducklib {
std::mutex Logger::stdout_mutex;

auto Logger::log_error(std::string_view text) -> void {
    return log(LogLevel::ERROR, text);
}

auto Logger::log_warn(std::string_view text) -> void {
    return log(LogLevel::WARNING, text);
}

auto Logger::log_info(std::string_view text) -> void {
    return log(LogLevel::INFO, text);
}

auto Logger::log_debug(std::string_view text) -> void {
    return log(LogLevel::DEBUG, text);
}

auto Logger::log(LogLevel level, std::string_view text) -> void {
    if (level > log_level_setting) {
        return;
    }

    auto message_buffer = std::array<char, MessageFormatBufferSize>{};
    auto prefix_length = write_prefix_to_message(message_buffer, level);
    auto end_it = std::ranges::copy(text, message_buffer.begin() + prefix_length).out;
    auto message_length = static_cast<size_t>(std::distance(message_buffer.begin(), end_it));
    auto final_message = std::string_view{ message_buffer.data(), message_length };

    // TODO: Support sinks/outputs
    stdout_sink(final_message);
}


auto Logger::write_prefix_to_message(const std::span<char>& buffer, LogLevel level) -> std::span<char>::size_type {
    constexpr auto prefixes = std::array<std::string_view, 4>{ "ERROR", "WARNING", "INFO", "DEBUG" };
    constexpr auto delimiter = std::string_view{ ": " };

    auto level_prefix = prefixes[static_cast<int>(level)];
    auto iterator = std::ranges::copy(level_prefix, buffer.begin()).out;
    iterator = std::ranges::copy(delimiter, iterator).out;

    return std::distance(buffer.begin(), iterator);
}

auto Logger::stdout_sink(std::string_view text) -> void {
    std::lock_guard lock{ stdout_mutex };
    std::cout << text.data() << std::endl;
}
}
