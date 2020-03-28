#include "window.hpp"

window::window() : window_interface<window>("Vulkan example", 960, 540), m_should_resize(false) {}

bool window::should_resize() {
    bool tmp = m_should_resize;
    m_should_resize = false;
    return tmp;
}

void window::on_key_down(sw::key_code code) {
    if (code == sw::key_code::e_F) {
        if (is_fullscreen()) {
            set_fullscreen(false);
            set_size(960, 540);
        }
        else {
            set_fullscreen(true);
        }
    }
}

void window::on_resize(uint32_t width, uint32_t height) { m_should_resize = true; }