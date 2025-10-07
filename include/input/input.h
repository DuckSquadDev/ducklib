#ifndef DUCKLIB_INPUT_H
#define DUCKLIB_INPUT_H

namespace ducklib {
struct InputState {
    static constexpr auto MAX_TEXT_INPUTS = 128;

    char text_inputs[MAX_TEXT_INPUTS];
    int text_input_count = 0;
    int mouse_x, mouse_y;
    
    bool mouse_buttons[5];
};

void register_raw_win_input();
void unregister_raw_win_input();
void process_win_input(InputState& input_state, uint32_t msg, WPARAM wParam, LPARAM lParam);
}

#endif //DUCKLIB_INPUT_H