#ifndef WIN_APP_WINDOW_H
#define WIN_APP_WINDOW_H
#include <string_view>
#include <Windows.h>

#include "core/app_window.h"

namespace ducklib {
class WinAppWindow final : AppWindow {
public:
    WinAppWindow(std::string_view title, int width, int height);
    ~WinAppWindow() override;

    auto process_messages() -> void override;
    auto close() -> void override;
    auto is_open() const -> bool override;
    auto type() const -> Type override;

    void register_message_callback(std::function<void(uint32_t, WPARAM, LPARAM)> callback);
    void process_message_callbacks(uint32_t msg, WPARAM w_param, LPARAM l_param);

    auto hwnd() const -> HWND;

private:
    HWND window_handle;
    std::vector<std::function<void(uint32_t, WPARAM, LPARAM)>> message_callbacks;

    static bool class_initialized;
    static constexpr auto DEFAULT_WINDOW_CLASS_NAME = "WinAppWindow";

    static auto register_window_class() -> void;
};
}

#endif //WIN_APP_WINDOW_H
