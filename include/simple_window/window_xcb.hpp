#pragma once
#include "simple_window/window_base.hpp"
#include "simple_window/enums.hpp"

#include <xcb/xcb.h>

namespace sw::detail {
    class window_xcb : public window_base {
    protected:
        window_xcb(const char* name, uint32_t width, uint32_t height);
        ~window_xcb();

    public:
        xcb_connection_t* get_connection() const { return m_connection; }
        xcb_window_t get_window() const { return m_window; }

        void set_size(uint32_t width, uint32_t height);
        void set_fullscreen(bool fullscreen);

        void lock_cursor();
        void unlock_cursor();
        void hide_cursor();
        void show_cursor();
        void set_cursor_image(cursor_icon cursor);

        void set_cursor_pos(const int32_t x, const int32_t y, const bool screenspace);

        std::string get_clipboard() const;
        void set_clipboard(const std::string& data);

        std::string get_name() const;
        void set_name(const std::string& name);

    private:
        inline xcb_intern_atom_reply_t* intern_atom_helper(bool only_if_exists, const char* str);

    protected:
        bool is_close_event(const xcb_generic_event_t* event) const;
        bool is_key_down_event(const xcb_generic_event_t* event,
                               const xcb_generic_event_t* prev) const;
        bool is_key_up_event(const xcb_generic_event_t* event,
                             const xcb_generic_event_t* next) const;

        key_code keycode_to_enum(const uint8_t code) const;
        mouse_code mousecode_to_enum(const uint8_t code) const;

    private:
        xcb_connection_t* m_connection;
        xcb_screen_t* m_screen;
        xcb_window_t m_window;
        xcb_intern_atom_reply_t* m_delete_window_atom;
    };
} // namespace sw::detail