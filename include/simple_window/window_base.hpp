#pragma once
#include <string>
#include <utility>
#include <stdexcept>

namespace sw::detail {
    class window_base {
    public:
        window_base(uint32_t width, uint32_t height) : m_width(width), m_height(height) {}

        inline bool is_open() const { return (0b1 & m_flags); }
        inline bool is_fullscreen() const { return (0b10 & m_flags); }
        inline bool is_cursor_locked() const { return (0b100 & m_flags); }

        inline uint32_t get_width() const { return m_width; }
        inline uint32_t get_height() const { return m_height; }

        inline int32_t get_mouse_x() const { return m_mouse_x; }
        inline int32_t get_mouse_y() const { return m_mouse_y; }
        inline std::pair<int32_t, int32_t> get_mouse_pos() const { return {m_mouse_x, m_mouse_y}; }

    protected:
        inline void set_open_flag_true() { m_flags |= 0b1; }
        inline void set_open_flag_false() { m_flags &= 0b11111110; }

        inline void set_fullscreen_flag_true() { m_flags |= 0b10; }
        inline void set_fullscreen_flag_false() { m_flags &= 0b11111101; }

        inline void set_cursor_flag_true() { m_flags |= 0b100; }
        inline void set_cursor_flag_false() { m_flags &= 0b11111011; }

    private:
        // 0b1: open, 0b10: fullscreen, 0b100: cursor_locked
        uint8_t m_flags = 0b1;

    protected:
        uint32_t m_width;
        uint32_t m_height;

        int32_t m_mouse_x = 0;
        int32_t m_mouse_y = 0;
        int32_t m_last_cursor_x = 0;
        int32_t m_last_cursor_y = 0;
    };
} // namespace sw::detail
