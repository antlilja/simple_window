#include "simple_window/window_xcb.hpp"

#include <cstdlib>
#include <cstring>

#include <xcb/xcb_cursor.h>

namespace sw::detail {
    window_xcb::window_xcb(const char* name, uint32_t width, uint32_t height)
        : window_base(width, height), m_connection(xcb_connect(nullptr, nullptr)) {
        if (xcb_connection_has_error(m_connection) > 0) {
            throw std::runtime_error("simple_window: Failed to make connection to xcb");
        }

        m_screen = xcb_setup_roots_iterator(xcb_get_setup(m_connection)).data;

        if (m_width == 0 || m_height == 0) {
            m_width = m_screen->width_in_pixels;
            m_height = m_screen->height_in_pixels;
        }

        // Window creation
        {
            uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
            // clang-format off
            uint32_t values[2] = {
                m_screen->white_pixel,

                XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY | 
                XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |  
                XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | 
                XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | 
                XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_FOCUS_CHANGE
            };
            // clang-format on

            m_window = xcb_generate_id(m_connection);

            xcb_create_window(m_connection, XCB_COPY_FROM_PARENT, m_window, m_screen->root, 10, 10,
                              m_width, m_height, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                              m_screen->root_visual, mask, values);
        }

        // Window delete event setup
        {
            auto reply = intern_atom_helper(true, "WM_PROTOCOLS");
            m_delete_window_atom = intern_atom_helper(false, "WM_DELETE_WINDOW");
            xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, m_window, (*reply).atom, 4, 32,
                                1, &(*m_delete_window_atom).atom);

            free(reply);
        }

        set_name(name);

        xcb_map_window(m_connection, m_window);
        xcb_flush(m_connection);
    }

    window_xcb::~window_xcb() {
        free(m_delete_window_atom);
        xcb_destroy_window(m_connection, m_window);
        xcb_disconnect(m_connection);
    }

    void window_xcb::set_size(const uint32_t width, const uint32_t height) {
        if (is_fullscreen()) {
            return;
        }

        xcb_unmap_window(m_connection, m_window);

        xcb_configure_window(m_connection, m_window, XCB_CONFIG_WINDOW_WIDTH, &width);

        xcb_configure_window(m_connection, m_window, XCB_CONFIG_WINDOW_HEIGHT, &height);

        xcb_map_window(m_connection, m_window);
        xcb_flush(m_connection);
    }

    void window_xcb::set_fullscreen(bool fullscreen) {
        if (is_fullscreen() == fullscreen) {
            return;
        }

        fullscreen == true ? set_fullscreen_flag_true() : set_fullscreen_flag_false();

        xcb_unmap_window(m_connection, m_window);

        if (fullscreen) {
            auto* atom_wm_state = intern_atom_helper(false, "_NET_WM_STATE");

            auto* atom_wm_fullscreen = intern_atom_helper(false, "_NET_WM_STATE_FULLSCREEN");

            xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, m_window, atom_wm_state->atom,
                                XCB_ATOM_ATOM, 32, 1, &(atom_wm_fullscreen->atom));

            free(atom_wm_fullscreen);
            free(atom_wm_state);

            m_width = m_screen->width_in_pixels;
            m_height = m_screen->height_in_pixels;
        }

        xcb_map_window(m_connection, m_window);
        xcb_flush(m_connection);

        xcb_unmap_window(m_connection, m_window);

        xcb_configure_window(m_connection, m_window, XCB_CONFIG_WINDOW_WIDTH, &m_width);

        xcb_configure_window(m_connection, m_window, XCB_CONFIG_WINDOW_HEIGHT, &m_height);

        xcb_map_window(m_connection, m_window);
        xcb_flush(m_connection);
    }

    void window_xcb::lock_cursor() {
        set_cursor_flag_true();

        m_mouse_x = m_last_cursor_x = m_width / 2;
        m_mouse_y = m_last_cursor_y = m_height / 2;

        set_cursor_pos(m_mouse_x, m_mouse_y, false);
    }

    void window_xcb::unlock_cursor() { set_cursor_flag_false(); }

    void window_xcb::hide_cursor() {
        // Create blank cursor
        xcb_cursor_t empty_cursor = xcb_generate_id(m_connection);
        {
            xcb_pixmap_t pix = xcb_generate_id(m_connection);

            // Create pixmap
            {
                auto cookie = xcb_create_pixmap_checked(m_connection, 1, pix, m_screen->root, 1, 1);

                if (auto* err = xcb_request_check(m_connection, cookie)) {
                    fprintf(stderr, "Cannot create pixmap: %d", err->error_code);
                    free(err);
                }
            }

            // Create cursor
            {
                auto cookie = xcb_create_cursor_checked(m_connection, empty_cursor, pix, pix, 0, 0,
                                                        0, 0, 0, 0, 0, 0);

                if (auto* err = xcb_request_check(m_connection, cookie)) {
                    fprintf(stderr, "Cannot create pixmap: %d", err->error_code);
                    free(err);
                }
            }

            xcb_free_pixmap(m_connection, pix);
        }

        // Set cursor
        xcb_change_window_attributes(m_connection, m_window, XCB_CW_CURSOR, (void*)(&empty_cursor));

        xcb_free_cursor(m_connection, empty_cursor);
    }

    void window_xcb::show_cursor() { set_cursor_image(cursor_icon::e_arrow); }

    void window_xcb::set_cursor_image(cursor_icon cursor) {
        if (xcb_cursor_context_t * context;
            xcb_cursor_context_new(m_connection, m_screen, &context) >= 0) {
            xcb_cursor_t cursor_image;

            switch (cursor) {
                case cursor_icon::e_arrow:
                    cursor_image = xcb_cursor_load_cursor(context, "arrow");
                    break;
                case cursor_icon::e_hand:
                    cursor_image = xcb_cursor_load_cursor(context, "hand1");
                    break;
                case cursor_icon::e_text:
                    cursor_image = xcb_cursor_load_cursor(context, "ibeam");
                    break;
                case cursor_icon::e_resize_all:
                    cursor_image = xcb_cursor_load_cursor(context, "size_all");
                    break;
                case cursor_icon::e_resize_EW:
                    cursor_image = xcb_cursor_load_cursor(context, "size_hor");
                    break;
                case cursor_icon::e_resize_NS:
                    cursor_image = xcb_cursor_load_cursor(context, "size_ver");
                    break;
                case cursor_icon::e_resize_NESW:
                    cursor_image = xcb_cursor_load_cursor(context, "size_bdiag");
                    break;
                case cursor_icon::e_resize_NWSE:
                    cursor_image = xcb_cursor_load_cursor(context, "size_fdiag");
                    break;
                case cursor_icon::e_loading:
                    cursor_image = xcb_cursor_load_cursor(context, "wait");
                    break;
            }

            xcb_change_window_attributes(m_connection, m_window, XCB_CW_CURSOR, &cursor_image);
            xcb_flush(m_connection);
            xcb_cursor_context_free(context);
        }
        else {
            // TODO: Handle error
        }
    }

    void window_xcb::set_cursor_pos(const int32_t x, const int32_t y, const bool screenspace) {
        xcb_warp_pointer(m_connection, XCB_NONE, screenspace ? XCB_NONE : m_window, 0, 0,
                         m_screen->width_in_pixels, m_screen->height_in_pixels, x, y);

        xcb_flush(m_connection);
    }

    std::string window_xcb::get_clipboard() const {
        return std::string();
        // TODO: implement
    }

    void window_xcb::set_clipboard(const std::string& data) {
        // TODO: implement
    }

    std::string window_xcb::get_name() const {
        auto cookie =
            xcb_get_property(m_connection, 0, m_window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0, 0);
        auto reply = xcb_get_property_reply(m_connection, cookie, nullptr);

        auto ptr = reinterpret_cast<char*>(xcb_get_property_value(reply));
        auto size = static_cast<size_t>(xcb_get_property_value_length(reply));
        std::string name(ptr, size);
        free(reply);
        return name;
    }

    void window_xcb::set_name(const std::string& name) {
        xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, m_window, XCB_ATOM_WM_NAME,
                            XCB_ATOM_STRING, 8, name.size(), name.c_str());
        xcb_flush(m_connection);
    }

    bool window_xcb::is_close_event(const xcb_generic_event_t* event) const {
        auto client_event = reinterpret_cast<const xcb_client_message_event_t*>(event);
        return client_event->data.data32[0] == m_delete_window_atom->atom;
    }

    bool window_xcb::is_key_down_event(const xcb_generic_event_t* event,
                                       const xcb_generic_event_t* prev) const {
        auto key_event = reinterpret_cast<const xcb_key_press_event_t*>(event);
        auto prev_event = reinterpret_cast<const xcb_key_release_event_t*>(prev);
        return (prev && ((prev->response_type & ~0x80) == XCB_KEY_RELEASE) &&
                (prev_event->detail == key_event->detail) && (prev_event->time == key_event->time));
    }

    bool window_xcb::is_key_up_event(const xcb_generic_event_t* event,
                                     const xcb_generic_event_t* next) const {
        auto key_event = reinterpret_cast<const xcb_key_press_event_t*>(event);
        auto next_event = reinterpret_cast<const xcb_key_release_event_t*>(next);
        return (!(next && ((next->response_type & ~0x80) == XCB_KEY_PRESS) &&
                  (next_event->detail == key_event->detail) &&
                  (next_event->time == key_event->time)));
    }

    inline xcb_intern_atom_reply_t* window_xcb::intern_atom_helper(bool only_if_exists,
                                                                   const char* str) {
        xcb_intern_atom_cookie_t cookie =
            xcb_intern_atom(m_connection, only_if_exists, std::strlen(str), str);
        return xcb_intern_atom_reply(m_connection, cookie, nullptr);
    }

    key_code window_xcb::keycode_to_enum(const uint8_t code) const {
        switch (code) {
            case 10: return key_code::e_1;
            case 11: return key_code::e_2;
            case 12: return key_code::e_3;
            case 13: return key_code::e_4;
            case 14: return key_code::e_5;
            case 15: return key_code::e_6;
            case 16: return key_code::e_7;
            case 17: return key_code::e_8;
            case 18: return key_code::e_9;
            case 19: return key_code::e_0;
            case 90: return key_code::e_numpad_0;
            case 87: return key_code::e_numpad_1;
            case 88: return key_code::e_numpad_2;
            case 89: return key_code::e_numpad_3;
            case 83: return key_code::e_numpad_4;
            case 84: return key_code::e_numpad_5;
            case 85: return key_code::e_numpad_6;
            case 79: return key_code::e_numpad_7;
            case 80: return key_code::e_numpad_8;
            case 81: return key_code::e_numpad_9;
            case 91: return key_code::e_numpad_decimal;
            case 86: return key_code::e_numpad_add;
            case 82: return key_code::e_numpad_subtract;
            case 63: return key_code::e_numpad_multiply;
            case 106: return key_code::e_numpad_divide;
            case 77: return key_code::e_numpad_lock;
            case 104: return key_code::e_numpad_enter;
            case 38: return key_code::e_A;
            case 56: return key_code::e_B;
            case 54: return key_code::e_C;
            case 40: return key_code::e_D;
            case 26: return key_code::e_E;
            case 41: return key_code::e_F;
            case 42: return key_code::e_G;
            case 43: return key_code::e_H;
            case 31: return key_code::e_I;
            case 44: return key_code::e_J;
            case 45: return key_code::e_K;
            case 46: return key_code::e_L;
            case 58: return key_code::e_M;
            case 57: return key_code::e_N;
            case 32: return key_code::e_O;
            case 33: return key_code::e_P;
            case 24: return key_code::e_Q;
            case 27: return key_code::e_R;
            case 39: return key_code::e_S;
            case 28: return key_code::e_T;
            case 30: return key_code::e_U;
            case 55: return key_code::e_V;
            case 25: return key_code::e_W;
            case 53: return key_code::e_X;
            case 29: return key_code::e_Y;
            case 52: return key_code::e_Z;
            case 111: return key_code::e_up;
            case 116: return key_code::e_down;
            case 114: return key_code::e_right;
            case 113: return key_code::e_left;
            case 60: return key_code::e_period;
            case 59: return key_code::e_comma;
            case 50: return key_code::e_left_shift;
            case 62: return key_code::e_right_shift;
            case 37: return key_code::e_left_ctrl;
            case 105: return key_code::e_right_ctrl;
            case 64: return key_code::e_left_alt;
            case 108: return key_code::e_right_alt;
            case 118: return key_code::e_insert;
            case 119: return key_code::e_delete;
            case 110: return key_code::e_home;
            case 115: return key_code::e_end;
            case 112: return key_code::e_page_up;
            case 117: return key_code::e_page_down;
            case 107: return key_code::e_print_screen;
            case 78: return key_code::e_scroll_lock;
            case 127: return key_code::e_pause;
            case 9: return key_code::e_escape;
            case 23: return key_code::e_tab;
            case 66: return key_code::e_caps_lock;
            case 133: return key_code::e_left_super;
            // case  : return key_code::e_right_super;
            case 65: return key_code::e_space;
            case 22: return key_code::e_backspace;
            case 36: return key_code::e_enter;
            case 135: return key_code::e_menu;
            case 61: return key_code::e_slash;
            case 51: return key_code::e_backslash;
            case 20: return key_code::e_minus;
            case 21: return key_code::e_equal;
            case 48: return key_code::e_apostrophe;
            case 47: return key_code::e_semicolon;
            case 34: return key_code::e_left_bracket;
            case 35: return key_code::e_right_bracket;
            case 49: return key_code::e_tilde;
            case 67: return key_code::e_F1;
            case 68: return key_code::e_F2;
            case 69: return key_code::e_F3;
            case 70: return key_code::e_F4;
            case 71: return key_code::e_F5;
            case 72: return key_code::e_F6;
            case 73: return key_code::e_F7;
            case 74: return key_code::e_F8;
            case 75: return key_code::e_F9;
            case 76: return key_code::e_F10;
            case 95: return key_code::e_F11;
            case 96: return key_code::e_F12;
            case 94: return key_code::e_OEM1;
            // case  : return key_code::e_OEM2;
            default: return key_code::e_NONE;
        }
    }

    mouse_code window_xcb::mousecode_to_enum(const uint8_t code) const {
        switch (code) {
            case 1: return mouse_code::e_left;
            case 2: return mouse_code::e_middle;
            case 3: return mouse_code::e_right;
            case 8: return mouse_code::e_1;
            case 9: return mouse_code::e_2;
            default: return mouse_code::e_NONE;
        }
    }
} // namespace sw::detail