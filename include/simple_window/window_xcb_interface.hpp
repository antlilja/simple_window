#pragma once
#include "simple_window/window_xcb.hpp"

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
            switch (curr->response_type & ~0x80) {
                // Destroy event
                case XCB_CLIENT_MESSAGE: {
                    if (is_close_event(curr)) {
                        set_open_flag_false();
                        if constexpr (has_on_close::value) {
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
                        if constexpr (has_on_resize::value) {
                            static_cast<Window*>(this)->on_resize(m_width, m_height);
                        }
                    }
                    break;
                }

                // Focus
                case XCB_FOCUS_IN: {
                    if constexpr (has_on_focus_in::value) {
                        static_cast<Window*>(this)->on_focus_in();
                    }
                    break;
                }

                case XCB_FOCUS_OUT: {
                    if constexpr (has_on_focus_out::value) {
                        if (is_open()) {
                            static_cast<Window*>(this)->on_focus_out();
                        }
                    }
                    break;
                }

                // Keyboard
                case XCB_KEY_PRESS: {
                    if constexpr (has_on_key_down::value) {
                        if (is_key_down_event(curr, prev)) {
                            auto key_event = reinterpret_cast<const xcb_key_press_event_t*>(curr);
                            static_cast<Window*>(this)->on_key_down(
                                keycode_to_enum(key_event->detail));
                        }
                    }
                    break;
                }

                case XCB_KEY_RELEASE: {
                    if constexpr (has_on_key_up::value) {
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
                            if constexpr (has_on_mouse_scroll_v::value) {
                                static_cast<Window*>(this)->on_mouse_scroll_v(1);
                            }
                            break;
                        }
                        case 5: {
                            if constexpr (has_on_mouse_scroll_v::value) {
                                static_cast<Window*>(this)->on_mouse_scroll_v(-1);
                            }
                            break;
                        }
                        case 6: {
                            if constexpr (has_on_mouse_scroll_h::value) {
                                static_cast<Window*>(this)->on_mouse_scroll_h(1);
                            }
                            break;
                        }
                        case 7: {
                            if constexpr (has_on_mouse_scroll_h::value) {
                                static_cast<Window*>(this)->on_mouse_scroll_h(-1);
                            }
                            break;
                        }
                        default: {
                            if constexpr (has_on_mouse_button_down::value) {
                                static_cast<Window*>(this)->on_mouse_button_down(
                                    mousecode_to_enum(button_event->detail), button_event->event_x,
                                    button_event->event_y);
                            }
                            break;
                        }
                    }
                    break;
                }

                case XCB_BUTTON_RELEASE: {
                    if constexpr (has_on_mouse_button_up::value) {
                        auto button_event =
                            reinterpret_cast<const xcb_button_release_event_t*>(curr);

                        // Check if scroll events
                        if (button_event->detail != 4 && button_event->detail != 5) {
                            static_cast<Window*>(this)->on_mouse_button_up(
                                mousecode_to_enum(button_event->detail), button_event->event_x,
                                button_event->event_y);
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

                    if constexpr (has_on_mouse_move_pos::value) {
                        static_cast<Window*>(this)->on_mouse_move_pos(m_mouse_x, m_mouse_y);
                    }

                    if constexpr (has_on_mouse_move_delta::value) {
                        static_cast<Window*>(this)->on_mouse_move_delta(dx, dy);
                    }
                    break;
                }
            }
        }

        class has_on_resize {
        private:
            typedef char YesType[1];
            typedef char NoType[2];

            template <typename C>
            static YesType& test(decltype(&C::on_resize));
            template <typename C>
            static NoType& test(...);

        public:
            enum { value = sizeof(test<Window>(0)) == sizeof(YesType) };
        };

        class has_on_move {
        private:
            typedef char YesType[1];
            typedef char NoType[2];

            template <typename C>
            static YesType& test(decltype(&C::on_move));
            template <typename C>
            static NoType& test(...);

        public:
            enum { value = sizeof(test<Window>(0)) == sizeof(YesType) };
        };

        class has_on_close {
        private:
            typedef char YesType[1];
            typedef char NoType[2];

            template <typename C>
            static YesType& test(decltype(&C::on_close));
            template <typename C>
            static NoType& test(...);

        public:
            enum { value = sizeof(test<Window>(0)) == sizeof(YesType) };
        };

        class has_on_focus_in {
        private:
            typedef char YesType[1];
            typedef char NoType[2];

            template <typename C>
            static YesType& test(decltype(&C::on_focus_in));
            template <typename C>
            static NoType& test(...);

        public:
            enum { value = sizeof(test<Window>(0)) == sizeof(YesType) };
        };

        class has_on_focus_out {
        private:
            typedef char YesType[1];
            typedef char NoType[2];

            template <typename C>
            static YesType& test(decltype(&C::on_focus_out));
            template <typename C>
            static NoType& test(...);

        public:
            enum { value = sizeof(test<Window>(0)) == sizeof(YesType) };
        };

        // Keyboard

        class has_on_key_down {
        private:
            typedef char YesType[1];
            typedef char NoType[2];

            template <typename C>
            static YesType& test(decltype(&C::on_key_down));
            template <typename C>
            static NoType& test(...);

        public:
            enum { value = sizeof(test<Window>(0)) == sizeof(YesType) };
        };

        class has_on_key_up {
        private:
            typedef char YesType[1];
            typedef char NoType[2];

            template <typename C>
            static YesType& test(decltype(&C::on_key_up));
            template <typename C>
            static NoType& test(...);

        public:
            enum { value = sizeof(test<Window>(0)) == sizeof(YesType) };
        };

        class has_on_char {
        private:
            typedef char YesType[1];
            typedef char NoType[2];

            template <typename C>
            static YesType& test(decltype(&C::on_char));
            template <typename C>
            static NoType& test(...);

        public:
            enum { value = sizeof(test<Window>(0)) == sizeof(YesType) };
        };

        // Mouse

        class has_on_mouse_button_down {
        private:
            typedef char YesType[1];
            typedef char NoType[2];

            template <typename C>
            static YesType& test(decltype(&C::on_mouse_button_down));
            template <typename C>
            static NoType& test(...);

        public:
            enum { value = sizeof(test<Window>(0)) == sizeof(YesType) };
        };

        class has_on_mouse_button_up {
        private:
            typedef char YesType[1];
            typedef char NoType[2];

            template <typename C>
            static YesType& test(decltype(&C::on_mouse_button_up));
            template <typename C>
            static NoType& test(...);

        public:
            enum { value = sizeof(test<Window>(0)) == sizeof(YesType) };
        };

        class has_on_mouse_scroll_v {
        private:
            typedef char YesType[1];
            typedef char NoType[2];

            template <typename C>
            static YesType& test(decltype(&C::on_mouse_scroll_v));
            template <typename C>
            static NoType& test(...);

        public:
            enum { value = sizeof(test<Window>(0)) == sizeof(YesType) };
        };

        class has_on_mouse_scroll_h {
        private:
            typedef char YesType[1];
            typedef char NoType[2];

            template <typename C>
            static YesType& test(decltype(&C::on_mouse_scroll_h));
            template <typename C>
            static NoType& test(...);

        public:
            enum { value = sizeof(test<Window>(0)) == sizeof(YesType) };
        };

        class has_on_mouse_move_pos {
        private:
            typedef char YesType[1];
            typedef char NoType[2];

            template <typename C>
            static YesType& test(decltype(&C::on_mouse_move_pos));
            template <typename C>
            static NoType& test(...);

        public:
            enum { value = sizeof(test<Window>(0)) == sizeof(YesType) };
        };

        class has_on_mouse_move_delta {
        private:
            typedef char YesType[1];
            typedef char NoType[2];

            template <typename C>
            static YesType& test(decltype(&C::on_mouse_move_delta));
            template <typename C>
            static NoType& test(...);

        public:
            enum { value = sizeof(test<Window>(0)) == sizeof(YesType) };
        };
    };
} // namespace sw