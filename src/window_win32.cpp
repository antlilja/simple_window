#include "simple_window/window_win32.hpp"

namespace sw::detail {
    window_win32::window_win32(const char* name, uint32_t width, uint32_t height, WNDPROC proc)
        : window_base(width, height) {
        static const wchar_t* class_name = L"simple_window_class";
        static bool class_exists = false;

        if (!class_exists) {
            WNDCLASS window_class = {0};
            window_class.lpfnWndProc = proc;
            window_class.hInstance = GetModuleHandle(NULL);
            window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
            window_class.lpszClassName = class_name;

            win32_assert(RegisterClass(&window_class));
            class_exists = true;
        }

        if (m_width == 0 || m_height == 0) {
            m_width = GetSystemMetrics(SM_CXSCREEN);
            m_height = GetSystemMetrics(SM_CYSCREEN);
        }

        RECT r;
        r.top = r.left = 0;
        r.right = m_width;
        r.bottom = m_height;

        adjust_window_rect(&r, WS_VISIBLE | WS_OVERLAPPEDWINDOW, false, NULL);

        const int32_t realWidth = r.right - r.left;
        const int32_t realHeight = r.bottom - r.top;

        m_handle = CreateWindowEx(NULL, class_name, multi_to_wide(name).c_str(),
                                  WS_VISIBLE | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                  realWidth, realHeight, NULL, NULL, GetModuleHandle(NULL), NULL);

        win32_assert(m_handle);

        SetWindowLongPtr(m_handle, GWLP_USERDATA, (LONG_PTR)this);
    }

    window_win32::~window_win32() { DestroyWindow(m_handle); }

    void window_win32::set_size(uint32_t width, uint32_t height) {
        if (is_fullscreen()) {
            return;
        }

        RECT r;
        r.top = r.left = 0;
        r.right = width;
        r.bottom = height;

        adjust_window_rect(&r, WS_VISIBLE | WS_OVERLAPPEDWINDOW, false, NULL);

        RECT wndRect;
        win32_assert(GetWindowRect(m_handle, &wndRect));

        win32_assert(SetWindowPos(m_handle, NULL, wndRect.left, wndRect.top, r.right - r.left,
                                  r.bottom - r.top, NULL));
    }

    void window_win32::set_fullscreen(bool fullscreen) {
        if (fullscreen == is_fullscreen()) {
            return;
        }

        fullscreen ? set_fullscreen_flag_true() : set_fullscreen_flag_false();

        DWORD style = WS_VISIBLE;
        uint32_t width;
        uint32_t height;
        uint32_t x = 0;
        uint32_t y = 0;
        if (fullscreen == false) {
            style |= WS_OVERLAPPEDWINDOW;

            RECT r;
            r.top = r.left = 0;

            adjust_window_rect(&r, style, FALSE, NULL);

            width = r.right - r.left;
            height = r.bottom - r.top;

            RECT window_rect;
            win32_assert(GetWindowRect(m_handle, &window_rect));
            x = window_rect.left;
            y = window_rect.top;
        }
        else {
            style |= WS_POPUP;

            DWORD flags = MONITOR_DEFAULTTOPRIMARY;
            auto monitor = MonitorFromWindow(m_handle, flags);

            MONITORINFO monitor_info = {sizeof(monitor_info)};
            win32_assert(GetMonitorInfo(monitor, &monitor_info));

            width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
            height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;
        }

        UINT flags = SWP_FRAMECHANGED;
        win32_assert(SetWindowPos(m_handle, NULL, x, y, width, height, flags));

        win32_assert(SetWindowLongPtr(m_handle, GWL_STYLE, (LONG_PTR)style));
    }

    void window_win32::lock_cursor() {
        RECT clip_rect;
        win32_assert(GetClientRect(m_handle, &clip_rect));
        win32_assert(ClientToScreen(m_handle, (POINT*)&clip_rect.left));
        win32_assert(ClientToScreen(m_handle, (POINT*)&clip_rect.right));
        win32_assert(ClipCursor(&clip_rect));

        set_cursor_flag_true();

        m_mouse_x = m_last_cursor_x = m_width / 2;
        m_mouse_y = m_last_cursor_y = m_height / 2;
        set_cursor_pos(m_mouse_x, m_mouse_y, false);
    }

    void window_win32::unlock_cursor() {
        win32_assert(ClipCursor(NULL));
        set_cursor_flag_false();
    }

    void window_win32::hide_cursor() { ShowCursor(FALSE); }

    void window_win32::show_cursor() { ShowCursor(TRUE); }

    void window_win32::set_cursor_image(cursor_icon icon) {
        switch (icon) {
            case cursor_icon::e_arrow: {
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                break;
            }
            case cursor_icon::e_text: {
                SetCursor(LoadCursor(NULL, IDC_IBEAM));
                break;
            }
            case cursor_icon::e_resize_all: {
                SetCursor(LoadCursor(NULL, IDC_SIZEALL));
                break;
            }
            case cursor_icon::e_resize_EW: {
                SetCursor(LoadCursor(NULL, IDC_SIZEWE));
                break;
            }
            case cursor_icon::e_resize_NS: {
                SetCursor(LoadCursor(NULL, IDC_SIZENS));
                break;
            }
            case cursor_icon::e_resize_NESW: {
                SetCursor(LoadCursor(NULL, IDC_SIZENESW));
                break;
            }
            case cursor_icon::e_resize_NWSE: {
                SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
                break;
            }
            case cursor_icon::e_hand: {
                SetCursor(LoadCursor(NULL, IDC_HAND));
                break;
            }
            case cursor_icon::e_loading: {
                SetCursor(LoadCursor(NULL, IDC_WAIT));
                break;
            }
        }
    }

    void window_win32::set_cursor_pos(int x, int y, bool screenspace) {
        POINT p = {x, y};
        if (!screenspace) {
            win32_assert(ClientToScreen(m_handle, &p));
        }
        win32_assert(SetCursorPos(p.x, p.y));
    }

    std::string window_win32::get_clipboard() const {
        win32_assert(OpenClipboard(m_handle));

        auto clipboard = GetClipboardData(CF_TEXT);

        std::string data(static_cast<const char*>(clipboard));
        win32_assert(CloseClipboard());
        return data;
    }

    void window_win32::set_clipboard(const std::string& data) {
        const auto alloc_size = sizeof(char) * (data.size() + 1);
        if (auto mem = GlobalAlloc(GMEM_FIXED, alloc_size); mem) {
            const auto copy_size = sizeof(char) * data.size();
            std::memcpy((void*)mem, data.data(), copy_size);
            GlobalUnlock(mem);

            win32_assert(OpenClipboard(m_handle));
            win32_assert(EmptyClipboard());
            win32_assert(SetClipboardData(CF_TEXT, mem));
            win32_assert(CloseClipboard());
        }
        else {
            win32_assert(false);
        }
    }

    std::string window_win32::get_name() const {
        std::wstring wstr;
        wstr.resize(GetWindowTextLength(m_handle));
        GetWindowText(m_handle, (LPWSTR)wstr.c_str(), static_cast<int32_t>(wstr.size()));
        return wide_to_multi(wstr);
    }

    void window_win32::set_name(const std::string& name) {
        auto wstr = multi_to_wide(name);
        win32_assert(SetWindowText(m_handle, wstr.c_str()));
    }

    void window_win32::poll_events() {
        MSG msg;
        while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    key_code window_win32::code_to_enum(const uint64_t code, const int64_t param) const {
        switch (code) {
            case 0x30: return key_code::e_0;
            case 0x31: return key_code::e_1;
            case 0x32: return key_code::e_2;
            case 0x33: return key_code::e_3;
            case 0x34: return key_code::e_4;
            case 0x35: return key_code::e_5;
            case 0x36: return key_code::e_6;
            case 0x37: return key_code::e_7;
            case 0x38: return key_code::e_8;
            case 0x39: return key_code::e_9;
            case 0x60: return key_code::e_numpad_0;
            case 0x61: return key_code::e_numpad_1;
            case 0x62: return key_code::e_numpad_2;
            case 0x63: return key_code::e_numpad_3;
            case 0x64: return key_code::e_numpad_4;
            case 0x65: return key_code::e_numpad_5;
            case 0x66: return key_code::e_numpad_6;
            case 0x67: return key_code::e_numpad_7;
            case 0x68: return key_code::e_numpad_8;
            case 0x69: return key_code::e_numpad_9;
            case 0x6E: return key_code::e_numpad_decimal;
            case 0x6B: return key_code::e_numpad_add;
            case 0x6D: return key_code::e_numpad_subtract;
            case 0x6A: return key_code::e_numpad_multiply;
            case 0x6F: return key_code::e_numpad_divide;
            case 0x90: return key_code::e_numpad_lock;
            case 0x41: return key_code::e_A;
            case 0x42: return key_code::e_B;
            case 0x43: return key_code::e_C;
            case 0x44: return key_code::e_D;
            case 0x45: return key_code::e_E;
            case 0x46: return key_code::e_F;
            case 0x47: return key_code::e_G;
            case 0x48: return key_code::e_H;
            case 0x49: return key_code::e_I;
            case 0x4A: return key_code::e_J;
            case 0x4B: return key_code::e_K;
            case 0x4C: return key_code::e_L;
            case 0x4D: return key_code::e_M;
            case 0x4E: return key_code::e_N;
            case 0x4F: return key_code::e_O;
            case 0x50: return key_code::e_P;
            case 0x51: return key_code::e_Q;
            case 0x52: return key_code::e_R;
            case 0x53: return key_code::e_S;
            case 0x54: return key_code::e_T;
            case 0x55: return key_code::e_U;
            case 0x56: return key_code::e_V;
            case 0x57: return key_code::e_W;
            case 0x58: return key_code::e_X;
            case 0x59: return key_code::e_Y;
            case 0x5A: return key_code::e_Z;
            case 0x26: return key_code::e_up;
            case 0x28: return key_code::e_down;
            case 0x27: return key_code::e_right;
            case 0x25: return key_code::e_left;
            case 0xBE: return key_code::e_period;
            case 0xBC: return key_code::e_comma;
            case 0x2D: return key_code::e_insert;
            case 0x2E: return key_code::e_delete;
            case 0x24: return key_code::e_home;
            case 0x23: return key_code::e_end;
            case 0x21: return key_code::e_page_up;
            case 0x22: return key_code::e_page_down;
            case 0x2C: return key_code::e_print_screen;
            case 0x91: return key_code::e_scroll_lock;
            case 0x13: return key_code::e_pause;
            case 0x1B: return key_code::e_escape;
            case 0x09: return key_code::e_tab;
            case 0x14: return key_code::e_caps_lock;
            case 0x5B: return key_code::e_left_super;
            case 0x5C: return key_code::e_right_super;
            case 0x20: return key_code::e_space;
            case 0x08: return key_code::e_backspace;
            case 0x5D: return key_code::e_menu;
            case 0xBF: return key_code::e_slash;
            case 0xDC: return key_code::e_backslash;
            case 0xBD: return key_code::e_minus;
            case 0xBB: return key_code::e_equal;
            case 0xDE: return key_code::e_apostrophe;
            case 0xBA: return key_code::e_semicolon;
            case 0xDB: return key_code::e_left_bracket;
            case 0xDD: return key_code::e_right_bracket;
            case 0xC0: return key_code::e_tilde;
            case 0x70: return key_code::e_F1;
            case 0x71: return key_code::e_F2;
            case 0x72: return key_code::e_F3;
            case 0x73: return key_code::e_F4;
            case 0x74: return key_code::e_F5;
            case 0x75: return key_code::e_F6;
            case 0x76: return key_code::e_F7;
            case 0x77: return key_code::e_F8;
            case 0x78: return key_code::e_F9;
            case 0x79: return key_code::e_F10;
            case 0x7A: return key_code::e_F11;
            case 0x7B: return key_code::e_F12;
            case 0xE2: return key_code::e_OEM1;
            case 0xDF: return key_code::e_OEM2;
            case 0x12:
                return ((param & 0x01000000) != 0) ? key_code::e_right_alt : key_code::e_left_alt;
            case 0x10:
                return MapVirtualKey((param & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX) == 0xA1
                           ? key_code::e_right_shift
                           : key_code::e_left_shift;
            case 0x11:
                return ((param & 0x01000000) != 0) ? key_code::e_right_ctrl : key_code::e_left_ctrl;
            case 0x0D:
                return ((param & 0x01000000) != 0) ? key_code::e_numpad_enter : key_code::e_enter;
            default: return key_code::e_NONE;
        }
    }

    std::string window_win32::wide_to_multi(const std::wstring& wstr) const {
        std::string str;

        const auto flags = WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK | WC_DEFAULTCHAR;
        str.resize(WideCharToMultiByte(CP_UTF8, flags, (LPWSTR)wstr.c_str(),
                                       static_cast<int32_t>(wstr.size()), (LPSTR)str.c_str(), 0,
                                       NULL, NULL));

        win32_assert(WideCharToMultiByte(CP_UTF8, flags, (LPWSTR)wstr.c_str(),
                                         static_cast<int32_t>(wstr.size()), (LPSTR)str.c_str(),
                                         static_cast<int32_t>(str.size()), NULL, NULL));
        return str;
    }

    std::wstring window_win32::multi_to_wide(const std::string& str) const {
        std::wstring wstr;
        wstr.resize(MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, str.data(),
                                        static_cast<int>(str.size()), (LPWSTR)wstr.c_str(), 0));

        win32_assert(MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, str.data(),
                                         static_cast<int>(str.size()), (LPWSTR)wstr.c_str(),
                                         static_cast<int>(wstr.size())));
        return wstr;
    }

    void window_win32::adjust_window_rect(RECT* rect, DWORD style, bool menu,
                                          DWORD ex_style) const {
        if (IsWindows10OrGreater()) {
            const auto dpi = GetDpiForWindow(m_handle);
            win32_assert(AdjustWindowRectExForDpi(rect, style, menu, ex_style, dpi));
        }
        else {
            win32_assert(AdjustWindowRectEx(rect, style, menu, ex_style));
        }
    }
} // namespace sw::detail