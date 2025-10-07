#ifndef APP_WINDOW_H
#define APP_WINDOW_H
#include <functional>

namespace ducklib {
class AppWindow {
public:
    enum class Type {
        WINDOWS
    };
    
    virtual ~AppWindow() {}

    virtual auto process_messages() -> void = 0;
    virtual auto close() -> void = 0;
    virtual auto is_open() const -> bool = 0;
    virtual auto type() const -> Type = 0;
};
}

#endif //APP_WINDOW_H
