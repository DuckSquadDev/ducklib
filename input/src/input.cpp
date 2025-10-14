#include <Windows.h>
#include <cstdint>
#include <stdexcept>
#include <array>
#include <span>
#include <cassert>

#include "input/input.h"

#include <iostream>
#include <print>

#include "core/unicode.h"

namespace ducklib {
void InputButtonState::clear() {
    for (auto i = 0; i < 5; ++i) {
        mouse_buttons[i] = false;
    }
}


std::array<RAWINPUTDEVICE, 2> get_raw_device_list(HWND hwnd) {
    std::array<RAWINPUTDEVICE, 2> raw_devices = {};

    raw_devices[0].usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
    raw_devices[0].usUsage = 0x02; // HID_USAGE_GENERIC_MOUSE
    raw_devices[0].hwndTarget = hwnd;

    raw_devices[1].usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
    raw_devices[1].usUsage = 0x06; // HID_USAGE_GENERIC_KEYBOARD
    raw_devices[1].hwndTarget = hwnd;

    return raw_devices;
}

bool InputState::mouse_pressed(uint8_t button) const {
    return current.mouse_buttons[button] && !previous.mouse_buttons[button];
}

bool InputState::mouse_released(uint8_t button) const {
    return !current.mouse_buttons[button] && previous.mouse_buttons[button];
}

void register_raw_win_input(HWND hwnd) {
    auto raw_devices = get_raw_device_list(hwnd);

    if (RegisterRawInputDevices(raw_devices.data(), 1, sizeof(raw_devices[0])) == FALSE) {
        throw new std::runtime_error("Failed to register for Windows raw input");
    }
}

void unregister_raw_win_input() {
    auto raw_devices = get_raw_device_list(nullptr);

    for (auto device : raw_devices) {
        device.dwFlags = RIDEV_REMOVE;
    }

    if (RegisterRawInputDevices(raw_devices.data(), 1, sizeof(raw_devices[0])) == FALSE) {
        throw new std::runtime_error("Failed to unregister from Windows raw input");
    }
}

RAWINPUT* allocate_and_read_raw_input(LPARAM lParam) {
    uint32_t raw_input_size = 0;
    if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &raw_input_size, sizeof(RAWINPUTHEADER)) == -1) {
        throw std::runtime_error("Failed to get raw input size from Windows");
    }

    auto raw_input_buffer = new std::byte[raw_input_size];

    if (GetRawInputData(
        reinterpret_cast<HRAWINPUT>(lParam),
        RID_INPUT,
        raw_input_buffer,
        &raw_input_size,
        sizeof(RAWINPUTHEADER)) != raw_input_size) {
        throw std::runtime_error("Failed to get raw input from Windows");
    }

    return reinterpret_cast<RAWINPUT*>(raw_input_buffer);
}

void check_mouse_button_down(InputState& input_state, uint8_t button, uint16_t button_flags, uint16_t event) {
    if (button_flags & event) {
        input_state.current.mouse_buttons[button] = true;
    }
}

void check_mouse_button_up(InputState& input_state, uint8_t button, uint16_t button_flags, uint16_t event) {
    if (button_flags & event) {
        input_state.current.mouse_buttons[button] = false;
    }
}

void process_win_input(AppWindow* app_window, InputState& input_state, uint32_t msg, WPARAM wParam, LPARAM lParam) {
    char8_t text_buffer[4];

    switch (msg) {
    case WM_CHAR: {
        auto code_units = cp_to_utf8(utf16_to_cp(reinterpret_cast<char16_t*>(&wParam), 2), text_buffer, 4);
        input_state.text_inputs.append_range(std::span(text_buffer, code_units));
        break;
    }
    case WM_SETCURSOR: {
        SetCursor(LoadCursor(nullptr, IDC_ARROW));
        break;
    }
    case WM_MOUSEMOVE: {
        input_state.mouse_x = lParam & 0xffff;
        input_state.mouse_y = (lParam >> 16) & 0xffff;
        break;
    }
    case WM_INPUT: {
        auto raw_input = allocate_and_read_raw_input(lParam);

        switch (raw_input->header.dwType) {
        case RIM_TYPEMOUSE: {
            auto& mouse_input = raw_input->data.mouse;

            check_mouse_button_down(input_state, 0, mouse_input.usButtonFlags, RI_MOUSE_BUTTON_1_DOWN);
            check_mouse_button_down(input_state, 1, mouse_input.usButtonFlags, RI_MOUSE_BUTTON_2_DOWN);
            check_mouse_button_down(input_state, 2, mouse_input.usButtonFlags, RI_MOUSE_BUTTON_3_DOWN);
            check_mouse_button_down(input_state, 3, mouse_input.usButtonFlags, RI_MOUSE_BUTTON_4_DOWN);
            check_mouse_button_down(input_state, 4, mouse_input.usButtonFlags, RI_MOUSE_BUTTON_5_DOWN);
            
            check_mouse_button_up(input_state, 0, mouse_input.usButtonFlags, RI_MOUSE_BUTTON_1_UP);
            check_mouse_button_up(input_state, 1, mouse_input.usButtonFlags, RI_MOUSE_BUTTON_2_UP);
            check_mouse_button_up(input_state, 2, mouse_input.usButtonFlags, RI_MOUSE_BUTTON_3_UP);
            check_mouse_button_up(input_state, 3, mouse_input.usButtonFlags, RI_MOUSE_BUTTON_4_UP);
            check_mouse_button_up(input_state, 4, mouse_input.usButtonFlags, RI_MOUSE_BUTTON_5_UP);

            if (mouse_input.usFlags == MOUSE_MOVE_RELATIVE) {
                // TODO: Do mouse movement stuff
            }

            // TODO: Test correctness for debugging (check the raw input flags and verify prev state matches it)
            break;
        }
        }
        break;
    }
    case WM_KILLFOCUS: {
        input_state.current.clear();
        break;
    }
    }
}

void frame_input_reset(InputState& input_state) {
    input_state.text_inputs.clear();
    input_state.previous = input_state.current;
}
}
