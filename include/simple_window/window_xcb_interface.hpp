#pragma once
#include "simple_window/window_xcb.hpp"
#include "simple_window/type_traits.hpp"

#include <cstdlib>

namespace sw {
    template <typename Window>
    class window_interface : public detail::window_xcb {
    protected:
        window_interface(const char* name, uint32_t width, uint32_t height)
            : window_xcb(name, width, height) {}

        void poll_events() {
            auto connection = get_connection();
            if (auto* curr = xcb_poll_for_event(connection); curr != nullptr) {
                xcb_generic_event_t* prev = nullptr;

                auto* next = xcb_poll_for_event(connection);
                while (next != nullptr) {
                    process_event(next, curr, prev);
                    free(prev);
                    prev = curr;
                    curr = next;
                    next = xcb_poll_for_event(connection);
                }
                process_event(next, curr, prev);
            }
        }

    private:
        void process_event(const xcb_generic_event_t* next, const xcb_generic_event_t* curr,
                           const xcb_generic_event_t* prev) {
            using namespace detail;
            switch (curr->response_type & ~0x80) {
                // Destroy event
                case XCB_CLIENT_MESSAGE: {
                    if (is_close_event(curr)) {
                        set_open_flag_false();
                        if constexpr (has_on_close<Window>::value) {
                            static_cast<Window*>(this)->on_close();
                        }
                    }
                    break;
                }

                // Resize event
                case XCB_CONFIGURE_NOTIFY: {
                    auto config_event = reinterpret_cast<const xcb_configure_notify_event_t*>(curr);
                    if (config_event->width != m_width || config_event->height != m_height) {
                        m_width = config_event->width;
                        m_height = config_event->height;
                        if constexpr (has_on_resize<Window>::value) {
                            static_cast<Window*>(this)->on_resize(m_width, m_height);
                        }
                    }
                    break;
                }

                // Focus
                case XCB_FOCUS_IN: {
                    if constexpr (has_on_focus_in<Window>::value) {
                        static_cast<Window*>(this)->on_focus_in();
                    }
                    break;
                }

                case XCB_FOCUS_OUT: {
                    if constexpr (has_on_focus_out<Window>::value) {
                        if (is_open()) {
                            static_cast<Window*>(this)->on_focus_out();
                        }
                    }
                    break;
                }

                // Keyboard
                case XCB_KEY_PRESS: {
                    if constexpr (has_on_key_down<Window>::value) {
                        if (is_key_down_event(curr, prev)) {
                            auto key_event = reinterpret_cast<const xcb_key_press_event_t*>(curr);
                            static_cast<Window*>(this)->on_key_down(
                                keycode_to_enum(key_event->detail));
                        }
                    }
                    break;
                }

                case XCB_KEY_RELEASE: {
                    if constexpr (has_on_key_up<Window>::value) {
                        if (is_key_up_event(curr, next)) {
                            auto key_event = reinterpret_cast<const xcb_key_release_event_t*>(curr);
                            static_cast<Window*>(this)->on_key_up(
                                keycode_to_enum(key_event->detail));
                        }
                    }
                    break;
                }

                // Mouse
                case XCB_BUTTON_PRESS: {
                    auto button_event = reinterpret_cast<const xcb_button_press_event_t*>(curr);
                    switch (button_event->detail) {
                        case 4: {
                            if constexpr (has_on_mouse_scroll_v<Window>::value) {
                                static_cast<Window*>(this)->on_mouse_scroll_v(1);
                            }
                            break;
                        }
                        case 5: {
                            if constexpr (has_on_mouse_scroll_v<Window>::value) {
                                static_cast<Window*>(this)->on_mouse_scroll_v(-1);
                            }
                            break;
                        }
                        case 6: {
                            if constexpr (has_on_mouse_scroll_h<Window>::value) {
                                static_cast<Window*>(this)->on_mouse_scroll_h(1);
                            }
                            break;
                        }
                        case 7: {
                            if constexpr (has_on_mouse_scroll_h<Window>::value) {
                                static_cast<Window*>(this)->on_mouse_scroll_h(-1);
                            }
                            break;
                        }
                        default: {
                            if constexpr (has_on_mouse_button_down<Window>::value) {
                                static_cast<Window*>(this)->on_mouse_button_down(
                                    mousecode_to_enum(button_event->detail));
                            }
                            break;
                        }
                    }
                    break;
                }

                case XCB_BUTTON_RELEASE: {
                    if constexpr (has_on_mouse_button_up<Window>::value) {
                        auto button_event =
                            reinterpret_cast<const xcb_button_release_event_t*>(curr);

                        // Check if scroll events
                        if (button_event->detail != 4 && button_event->detail != 5) {
                            static_cast<Window*>(this)->on_mouse_button_up(
                                mousecode_to_enum(button_event->detail));
                        }
                    }
                    break;
                }

                case XCB_LEAVE_NOTIFY: {
                    if (is_cursor_locked()) {
                        m_last_cursor_x = static_cast<int32_t>(m_width / 2);
                        m_last_cursor_y = static_cast<int32_t>(m_height / 2);
                        set_cursor_pos(m_last_cursor_x, m_last_cursor_y, false);
                    }
                }

                case XCB_MOTION_NOTIFY: {
                    auto motion_event = reinterpret_cast<const xcb_motion_notify_event_t*>(curr);

                    const auto x = static_cast<int32_t>(motion_event->event_x);
                    const auto y = static_cast<int32_t>(motion_event->event_y);
                    const auto dx = x - m_last_cursor_x;
                    const auto dy = y - m_last_cursor_y;
                    m_mouse_x += dx;
                    m_mouse_y += dy;
                    m_last_cursor_x = x;
                    m_last_cursor_y = y;

                    if constexpr (has_on_mouse_move_pos<Window>::value) {
                        static_cast<Window*>(this)->on_mouse_move_pos(m_mouse_x, m_mouse_y);
                    }

                    if constexpr (has_on_mouse_move_delta<Window>::value) {
                        static_cast<Window*>(this)->on_mouse_move_delta(dx, dy);
                    }
                    break;
                }
            }
        }
    };
} // namespace sw