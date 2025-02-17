#include "core/win/win_app_window.h"

namespace ducklib {
auto __stdcall win_app_window_proc(HWND window, unsigned int msg, WPARAM w_param, LPARAM l_param) -> LRESULT;

bool WinAppWindow::class_initialized = false;

WinAppWindow::WinAppWindow(std::string_view title, int width, int height) {
    if (!class_initialized) {
        register_window_class();
    }
    
    window_handle = CreateWindowEx(
        0,
        DEFAULT_WINDOW_CLASS_NAME,
        title.data(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr);

    SetWindowLongPtr(window_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
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

auto WinAppWindow::hwnd() const -> HWND {
    return window_handle;
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
