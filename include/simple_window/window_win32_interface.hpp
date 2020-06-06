#pragma once
#include "simple_window/window_win32.hpp"

namespace sw {
    template <typename Window>
    class window_interface : public detail::window_win32 {
    protected:
        window_interface(const char* name, uint32_t width, uint32_t height)
            : window_win32(name, width, height, &window_proc) {}

    private:
        static LRESULT CALLBACK window_proc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
            if (auto ptr = GetWindowLongPtr(window, GWLP_USERDATA)) {
                auto wnd = reinterpret_cast<window_interface<Window>*>(ptr);
                return wnd->window_callback(window, msg, wParam, lParam);
            }
            return DefWindowProc(window, msg, wParam, lParam);
        }

        LRESULT window_callback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
            switch (msg) {
                // Keyboard
                case WM_SYSKEYDOWN:
                case WM_KEYDOWN: {
                    if constexpr (has_on_key_down::value) {
                        if (!(lParam & 0x40000000)) {
                            static_cast<Window*>(this)->on_key_down(code_to_enum(
                                static_cast<uint64_t>(wParam), static_cast<int64_t>(lParam)));
                        }
                    }
                    break;
                }
                case WM_SYSKEYUP:
                case WM_KEYUP: {
                    if constexpr (has_on_key_up::value) {
                        static_cast<Window*>(this)->on_key_up(code_to_enum(
                            static_cast<uint64_t>(wParam), static_cast<int64_t>(lParam)));
                    }
                    break;
                }

                case WM_CHAR: {
                    if constexpr (has_on_char::value) {
                        if (wParam > 0 && wParam < 0x10000) {
                            static_cast<Window*>(this)->on_char(static_cast<wchar_t>(wParam));
                        }
                    }
                    break;
                }

                // Mouse
                case WM_LBUTTONDOWN: handle_mouse_down_event(mouse_code::e_left, lParam); break;
                case WM_LBUTTONUP: handle_mouse_up_event(mouse_code::e_left, lParam); break;

                case WM_MBUTTONDOWN: handle_mouse_down_event(mouse_code::e_middle, lParam); break;
                case WM_MBUTTONUP: handle_mouse_up_event(mouse_code::e_middle, lParam); break;

                case WM_RBUTTONDOWN: handle_mouse_down_event(mouse_code::e_right, lParam); break;
                case WM_RBUTTONUP: handle_mouse_up_event(mouse_code::e_right, lParam); break;

                case WM_XBUTTONDOWN: {
                    if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) {
                        handle_mouse_down_event(mouse_code::e_1, lParam);
                    }
                    else {
                        handle_mouse_down_event(mouse_code::e_2, lParam);
                    }
                    break;
                }

                case WM_XBUTTONUP: {
                    if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) {
                        handle_mouse_up_event(mouse_code::e_1, lParam);
                    }
                    else {
                        handle_mouse_up_event(mouse_code::e_2, lParam);
                    }
                    break;
                }

                case WM_MOUSEWHEEL: {
                    if constexpr (has_on_mouse_scroll_v::value) {
                        static_cast<Window*>(this)->on_mouse_scroll_v(
                            GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
                    }
                    break;
                }

                case WM_MOUSEHWHEEL: {
                    if constexpr (has_on_mouse_scroll_h::value) {
                        static_cast<Window*>(this)->on_mouse_scroll_h(
                            GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
                    }
                    break;
                }

                case WM_MOUSELEAVE: {
                    if (is_cursor_locked()) {
                        m_last_cursor_x = static_cast<int32_t>(m_width / 2);
                        m_last_cursor_y = static_cast<int32_t>(m_height / 2);
                        set_cursor_pos(m_last_cursor_x, m_last_cursor_y, false);
                    }
                    break;
                }

                case WM_MOUSEMOVE: {
                    handle_mouse_move(static_cast<int32_t>(LOWORD(lParam)), static_cast<int32_t>(HIWORD(lParam)));

                    if constexpr (has_on_mouse_move_pos::value) {
                        static_cast<Window*>(this)->on_mouse_move_pos(m_mouse_x, m_mouse_y);
                    }

                    if constexpr (has_on_mouse_move_delta::value) {
                        static_cast<Window*>(this)->on_mouse_move_delta(m_mouse_x - m_last_cursor_x, m_mouse_y - m_last_cursor_y);
                    }
                    break;
                }

                // System
                case WM_SIZE: {
                    m_width = static_cast<uint32_t>(LOWORD(lParam));
                    m_height = static_cast<uint32_t>(HIWORD(lParam));
                    if constexpr (has_on_resize::value) {
                        static_cast<Window*>(this)->on_resize(m_width, m_height);
                    }
                    break;
                }

                case WM_MOVE: {
                    if constexpr (has_on_move::value) {
                        static_cast<Window*>(this)->on_move((int)(short)LOWORD(lParam),
                                                            (int)(short)HIWORD(lParam));
                    }
                    break;
                }

                case WM_CLOSE: {
                    set_open_flag_false();
                    if constexpr (has_on_close::value) {
                        static_cast<Window*>(this)->on_close();
                    }
                    return 0;
                }

                case WM_SETFOCUS: {
                    if constexpr (has_on_focus_in::value) {
                        static_cast<Window*>(this)->on_focus_in();
                    }
                    break;
                }

                case WM_KILLFOCUS: {
                    if constexpr (has_on_focus_out::value) {
                        if (is_open()) {
                            static_cast<Window*>(this)->on_focus_out();
                        }
                    }
                    break;
                }
            }

            return DefWindowProc(window, msg, wParam, lParam);
        }

        inline void handle_mouse_down_event(const mouse_code code, LPARAM lparam) {
            if constexpr (has_on_mouse_button_down::value) {
                const auto x = static_cast<int32_t>(LOWORD(lparam));
                const auto y = static_cast<int32_t>(HIWORD(lparam));
                static_cast<Window*>(this)->on_mouse_button_down(code, x, y);
            }
        }

        inline void handle_mouse_up_event(const mouse_code code, LPARAM lparam) {
            if constexpr (has_on_mouse_button_up::value) {
                const auto x = static_cast<int32_t>(LOWORD(lparam));
                const auto y = static_cast<int32_t>(HIWORD(lparam));
                static_cast<Window*>(this)->on_mouse_button_up(code, x, y);
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