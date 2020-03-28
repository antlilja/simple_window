#pragma once
#include <simple_window/simple_window.hpp>

class window : public sw::window_interface<window> {
    friend class sw::window_interface<window>;

public:
    window();

    bool should_resize();

    void update() { poll_events(); }

private:
    void on_key_down(sw::key_code code);
    void on_resize(uint32_t width, uint32_t height);

private:
    bool m_should_resize;
};