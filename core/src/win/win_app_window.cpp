#include "core/win/win_app_window.h"

namespace ducklib {
auto __stdcall win_app_window_proc(HWND window, unsigned int msg, WPARAM w_param, LPARAM l_param) -> LRESULT;

bool WinAppWindow::class_initialized = false;

WinAppWindow::WinAppWindow(std::string_view title, int width, int height) {
    if (!class_initialized) {
        register_window_class();
    }

    caption_height = GetSystemMetrics(SM_CYCAPTION);
    frame_border_size = GetSystemMetrics(SM_CXSIZEFRAME);
    RECT adjusted_rect = { 0, 0, width, height };
    auto window_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    
    if (AdjustWindowRectEx(&adjusted_rect, window_style, FALSE, 0) == 0) {
        std::abort();
    }

    // Manual adjustment because AdjustWindowRect() is wrong
    adjusted_rect.right += 8; // 4 pixel wide borders
    adjusted_rect.bottom += 31; // Don't even know the exact ones as caption can be handled but WS_OVERLAPPEDWINDOW just doesn't work
    
    window_handle = CreateWindowEx(
        0,
        DEFAULT_WINDOW_CLASS_NAME,
        title.data(),
        window_style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        adjusted_rect.right,
        adjusted_rect.bottom,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr);

    SetWindowLongPtr(window_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    RECT client_rect = {};
    GetClientRect(window_handle, &client_rect);
    return;
}

WinAppWindow::~WinAppWindow() {
    DestroyWindow(window_handle);
}

auto WinAppWindow::process_messages() -> void {
    auto message = MSG{};

    while (PeekMessage(&message, window_handle, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

auto WinAppWindow::close() -> void {
    DestroyWindow(window_handle);
    window_handle = nullptr;
}

auto WinAppWindow::is_open() const -> bool {
    return window_handle != nullptr;
}

auto WinAppWindow::type() const -> Type {
    return Type::WINDOWS;
}

void WinAppWindow::register_message_callback(std::function<void(AppWindow*, uint32_t, WPARAM, LPARAM)> callback) {
    message_callbacks.push_back(callback);
}

void WinAppWindow::process_message_callbacks(uint32_t msg, WPARAM w_param, LPARAM l_param) {
    for (auto callback : message_callbacks) {
        if (callback) {
            callback(this, msg, w_param, l_param);
        }
    }
}

auto WinAppWindow::hwnd() const -> HWND {
    return window_handle;
}

auto WinAppWindow::border_size() const -> uint32_t {
    return frame_border_size;
}

auto WinAppWindow::title_bar_height() const -> uint32_t {
    return caption_height;
}

auto WinAppWindow::register_window_class() -> void {
    auto window_class = WNDCLASSEXA{
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = win_app_window_proc,
        .hInstance = GetModuleHandle(nullptr),
        .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
        .lpszClassName = DEFAULT_WINDOW_CLASS_NAME
    };

    RegisterClassExA(&window_class);
    class_initialized = true;
}

auto __stdcall win_app_window_proc(HWND window, unsigned int msg, WPARAM w_param, LPARAM l_param) -> LRESULT {
    auto app_window = reinterpret_cast<WinAppWindow*>(GetWindowLongPtr(window, GWLP_USERDATA));

    if (!app_window) {
        return DefWindowProc(window, msg, w_param, l_param);
    }
    
    app_window->process_message_callbacks(msg, w_param, l_param);

    switch (msg) {
    case WM_CLOSE:
        app_window->close();
        return 0;
    case WM_DESTROY:
        // TODO: Reconsider?
        // PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(window, msg, w_param, l_param);
    }
}
}
