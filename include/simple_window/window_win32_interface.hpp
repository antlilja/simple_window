#pragma once
#include "simple_window/window_win32.hpp"
#include "simple_window/type_traits.hpp"

namespace sw {
    template <typename Window>
    class window_interface : public detail::window_win32 {
    protected:
        window_interface(const char* name, uint32_t width, uint32_t height)
            : window_win32<Window>(name, width, height, window_callback) {}

    private:
        static LRESULT window_callback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
            using namespace detail;
            switch (msg) {
                // Keyboard
                case WM_SYSKEYDOWN:
                case WM_KEYDOWN: {
                    if constexpr (has_on_key_down<Window>::value) {
                        if (!(lParam & 0x40000000)) {
                            static_cast<Window*>(this)->on_key_down(code_to_enum(
                                static_cast<uint64_t>(wParam), static_cast<int64_t>(lParam)));
                        }
                    }
                    break;
                }
                case WM_SYSKEYUP:
                case WM_KEYUP: {
                    if constexpr (has_on_key_up<Window>::value) {
                        static_cast<Window*>(this)->on_key_up(code_to_enum(
                            static_cast<uint64_t>(wParam), static_cast<int64_t>(lParam)));
                    }
                    break;
                }

                case WM_CHAR: {
                    if constexpr (has_on_char<Window>::value) {
                        if (wParam > 0 && wParam < 0x10000) {
                            static_cast<Window*>(this)->on_char(static_cast<wchar_t>(wParam));
                        }
                    }
                    break;
                }

                // Mouse
                case WM_LBUTTONDOWN: handle_mouse_down_event(mouse_code::e_left); break;
                case WM_LBUTTONUP: handle_mouse_up_event(mouse_code::e_left); break;

                case WM_MBUTTONDOWN: handle_mouse_down_event(mouse_code::e_middle); break;
                case WM_MBUTTONUP: handle_mouse_up_event(mouse_code::e_middle); break;

                case WM_RBUTTONDOWN: handle_mouse_down_event(mouse_code::e_right); break;
                case WM_RBUTTONUP: handle_mouse_up_event(mouse_code::e_right); break;

                case WM_XBUTTONDOWN: {
                    if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) {
                        handle_mouse_down_event(mouse_code::e_1);
                    }
                    else {
                        handle_mouse_down_event(mouse_code::e_2);
                    }
                    break;
                }

                case WM_XBUTTONUP: {
                    if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) {
                        handle_mouse_up_event(mouse_code::e_1);
                    }
                    else {
                        handle_mouse_up_event(mouse_code::e_2);
                    }
                    break;
                }

                case WM_MOUSEWHEEL: {
                    if constexpr (has_on_mouse_scroll_v<Window>::value) {
                        static_cast<Window*>(this)->on_mouse_scroll_v(
                            GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
                    }
                    break;
                }

                case WM_MOUSEHWHEEL: {
                    if constexpr (has_on_mouse_scroll_h<Window>::value) {
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
                    const auto x = static_cast<int32_t>(GET_X_LPARAM(lParam));
                    const auto y = static_cast<int32_t>(GET_Y_LPARAM(lParam));
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

                // System
                case WM_SIZE: {
                    m_width = static_cast<uint32_t>(LOWORD(lParam));
                    m_height = static_cast<uint32_t>(HIWORD(lParam));
                    if constexpr (has_on_resize<Window>::value) {
                        static_cast<Window*>(this)->on_resize(m_width, m_height);
                    }
                    break;
                }

                case WM_MOVE: {
                    if constexpr (has_on_move<Window>::value) {
                        static_cast<Window*>(this)->on_move((int)(short)LOWORD(lParam),
                                                            (int)(short)HIWORD(lParam));
                    }
                    break;
                }

                case WM_CLOSE: {
                    m_open = false;
                    if constexpr (has_on_close<Window>::value) {
                        static_cast<Window*>(this)->on_close();
                    }
                    return 0;
                }

                case WM_SETFOCUS: {
                    if constexpr (has_on_focus_in<Window>::value) {
                        static_cast<Window*>(this)->on_focus_in();
                    }
                    break;
                }

                case WM_KILLFOCUS: {
                    if constexpr (has_on_focus_out<Window>::value) {
                        if (m_open) {
                            static_cast<Window*>(this)->on_focus_out();
                        }
                    }
                    break;
                }
            }

            return DefWindowProc(window, msg, wParam, lParam);
        }

        inline void handle_mouse_down_event(const mouse_code code) {
            if constexpr (detail::has_on_mouse_button_down<Window>::value) {
                static_cast<Window*>(this)->on_mouse_button_down(code);
            }
        }

        inline void handle_mouse_up_event(const mouse_code code) {
            if constexpr (detail::has_on_mouse_button_up<Window>::value) {
                static_cast<Window*>(this)->on_mouse_button_up(code);
            }
        }
    };
} // namespace sw