#pragma once
#include <cstdint>

namespace sw {
    enum class cursor_icon : std::uint8_t {
        e_arrow,
        e_hand,
        e_text,
        e_resize_all,
        e_resize_EW,
        e_resize_NS,
        e_resize_NESW,
        e_resize_NWSE,
        e_loading
    };

    enum class key_code : std::uint8_t {
        e_0,
        e_1,
        e_2,
        e_3,
        e_4,
        e_5,
        e_6,
        e_7,
        e_8,
        e_9,
        e_numpad_0,
        e_numpad_1,
        e_numpad_2,
        e_numpad_3,
        e_numpad_4,
        e_numpad_5,
        e_numpad_6,
        e_numpad_7,
        e_numpad_8,
        e_numpad_9,
        e_numpad_decimal,
        e_numpad_add,
        e_numpad_subtract,
        e_numpad_multiply,
        e_numpad_divide,
        e_numpad_lock,
        e_numpad_enter,
        e_A,
        e_B,
        e_C,
        e_D,
        e_E,
        e_F,
        e_G,
        e_H,
        e_I,
        e_J,
        e_K,
        e_L,
        e_M,
        e_N,
        e_O,
        e_P,
        e_Q,
        e_R,
        e_S,
        e_T,
        e_U,
        e_V,
        e_W,
        e_X,
        e_Y,
        e_Z,
        e_up,
        e_down,
        e_right,
        e_left,
        e_period,
        e_comma,
        e_left_shift,
        e_right_shift,
        e_left_ctrl,
        e_right_ctrl,
        e_left_alt,
        e_right_alt,
        e_insert,
        e_delete,
        e_home,
        e_end,
        e_page_up,
        e_page_down,
        e_print_screen,
        e_scroll_lock,
        e_pause,
        e_escape,
        e_tab,
        e_caps_lock,
        e_left_super,
        e_right_super,
        e_space,
        e_backspace,
        e_enter,
        e_menu,
        e_slash,
        e_backslash,
        e_minus,
        e_equal,
        e_apostrophe,
        e_semicolon,
        e_left_bracket,
        e_right_bracket,
        e_tilde,
        e_F1,
        e_F2,
        e_F3,
        e_F4,
        e_F5,
        e_F6,
        e_F7,
        e_F8,
        e_F9,
        e_F10,
        e_F11,
        e_F12,
        e_OEM1,
        e_OEM2,
        e_MAX_KEYS,
        e_NONE
    };

    enum class mouse_code : std::uint8_t {
        e_left,
        e_right,
        e_middle,
        e_1,
        e_2,
        e_MAX_BUTTONS,
        e_NONE
    };
} // namespace sw