#ifndef DUCKLIB_LOG_LEVEL_H
#define DUCKLIB_LOG_LEVEL_H
#include <cstdint>

#undef ERROR

namespace ducklib {
enum class LogLevel : uint8_t {
    OFF,
    FATAL,
    ERROR,
    WARNING,
    INFO,
    DEBUG,
    TRACE
};
}

#endif //DUCKLIB_LOG_LEVEL_H
