#pragma once
#include "simple_window/window_base.hpp"
#include "simple_window/enums.hpp"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#include <VersionHelpers.h>

namespace sw::detail {
    class window_win32 : public window_base {
    protected:
        template <typename WindowClass>
        window_win32(const char* name, uint32_t width, uint32_t height, WNDPROC proc)
            : window_base(width, height) {
            static std::wstring class_name = multi_to_wide(std::string(typeid(WindowClass).name()));
            static bool class_exists = false;

            if (!class_exists) {
                class_name = WNDCLASS window_class = {0};
                window_class.lpfnWndProc = proc;
                window_class.hInstance = GetModuleHandle(NULL);
                window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
                window_class.lpszClassName = class_name.c_str();

                win32_assert(RegisterClass(&window_class));
                class_exists = true;
            }

            create_window(name, class_name);
        }

        ~window_win32();

        void poll_events();
        key_code code_to_enum(const uint64_t code, const int64_t param) const;

    public:
        HWND get_hwnd() const { return m_handle; }
        HINSTANCE get_hinstance() const { return GetModuleHandle(NULL); }

        void set_size(uint32_t width, uint32_t height);
        void set_fullscreen(bool fullscreen);

        void lock_cursor();
        void unlock_cursor();

        void hide_cursor();
        void show_cursor();

        void set_cursor_image(cursor_icon icon);

        void set_cursor_pos(int x, int y, bool screenspace);

        std::string get_clipboard() const;
        void set_clipboard(const std::string& data);

        std::string get_name() const;
        void set_name(const std::string& name);

    private:
        void create_window(const char* name);

        std::string wide_to_multi(const std::wstring& wstr) const;

        std::wstring multi_to_wide(const std::string& str) const;

        void adjust_window_rect(RECT* rect, DWORD style, bool menu, DWORD ex_style) const;

        template <typename ErrorCode>
        void win32_assert(ErrorCode error_code) {
#ifdef SW_DEBUG
            if (!error_code) {
                const auto err = GetLastError();

                const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                    FORMAT_MESSAGE_IGNORE_INSERTS;

                const DWORD lang_id = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

                LPSTR message_buffer = nullptr;
                const size_t size = FormatMessageA(flags, nullptr, err, lang_id,
                                                   (LPSTR)&message_buffer, 0, nullptr);

                const std::string message(message_buffer, size);
                LocalFree(message_buffer);

                throw std::runtime_error("simple_window: Win32: " + message);
            }
#endif
        }

    protected:
        HWND m_handle;
    };
} // namespace sw::detail