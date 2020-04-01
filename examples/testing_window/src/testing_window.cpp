#include <simple_window/simple_window.hpp>

#include <iostream>

class window final : public sw::window_interface<window> {
    friend class sw::window_interface<window>;

public:
    window() : window_interface<window>("Testing window", 960, 540) {}

    void update() { poll_events(); }

protected:
    void on_key_down(const sw::key_code code) {
        switch (code) {
            case sw::key_code::e_1: set_cursor_image(sw::cursor_icon::e_arrow); break;
            case sw::key_code::e_2: set_cursor_image(sw::cursor_icon::e_hand); break;
            case sw::key_code::e_3: set_cursor_image(sw::cursor_icon::e_text); break;
            case sw::key_code::e_4: set_cursor_image(sw::cursor_icon::e_resize_all); break;
            case sw::key_code::e_5: set_cursor_image(sw::cursor_icon::e_resize_EW); break;
            case sw::key_code::e_6: set_cursor_image(sw::cursor_icon::e_resize_NS); break;
            case sw::key_code::e_7: set_cursor_image(sw::cursor_icon::e_resize_NESW); break;
            case sw::key_code::e_8: set_cursor_image(sw::cursor_icon::e_resize_NWSE); break;
            case sw::key_code::e_9: set_cursor_image(sw::cursor_icon::e_loading); break;
            case sw::key_code::e_space:
                lock_cursor();
                // hide_cursor();
                break;
            case sw::key_code::e_U:
                unlock_cursor();
                // show_cursor();
                break;
            default: break;
        }
    }

    void on_move(const int x, const int y) {
        std::cout << "Window moved: " << x << ", " << y << '\n';
    }
    void on_mouse_button_down(sw::mouse_code code, int x, int y) {
        if (code == sw::mouse_code::e_left) {
            std::cout << x << ", " << y << '\n';
        }
    }

    void on_mouse_scroll_v(int delta) { std::cout << "Mouse scroll: " << delta << '\n'; }

    void on_resize(uint32_t width, uint32_t height) {
        std::cout << "Window resize: " << width << ", " << height << '\n';
    }

    void on_close() { std::cout << "Close\n"; }

    void on_focus(bool focus) { std::cout << "Focus: " << focus << '\n'; }
};

int main() {
    window window;

    while (window.is_open()) {
        window.update();
    }
}