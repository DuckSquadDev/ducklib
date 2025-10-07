#include <Windows.h>
#include <cstdint>
#include <stdexcept>
#include <array>

#include "input/input.h"

namespace ducklib {
void process_win_input(InputState& input_state, uint32_t msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CHAR: break;
    case WM_MOUSEMOVE: break;
    case WM_LBUTTONDOWN: break;
    case WM_KEYDOWN: break;
    }
}

std::array<RAWINPUTDEVICE, 2> get_raw_device_list() {
    std::array<RAWINPUTDEVICE, 2> raw_devices = {};

    raw_devices[0].usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
    raw_devices[0].usUsage = 0x02; // HID_USAGE_GENERIC_MOUSE

    return raw_devices;
}

void register_raw_win_input() {
    auto raw_devices = get_raw_device_list();

    if (RegisterRawInputDevices(raw_devices.data(), 1, sizeof(raw_devices[0])) == FALSE) {
        throw new std::runtime_error("Failed to register for Windows raw input");
    }

    
}

void unregister_raw_win_input() {
    auto raw_devices = get_raw_device_list();

    for (auto device : raw_devices) {
        device.dwFlags = RIDEV_REMOVE;
    }
    
    if (RegisterRawInputDevices(raw_devices.data(), 1, sizeof(raw_devices[0])) == FALSE) {
        throw new std::runtime_error("Failed to unregister from Windows raw input");
    }
}
}
