#pragma once

namespace sw::detail {
    template <typename T>
    class has_on_resize {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C>
        static YesType& test(decltype(&C::on_resize));
        template <typename C>
        static NoType& test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };

    template <typename T>
    class has_on_move {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C>
        static YesType& test(decltype(&C::on_move));
        template <typename C>
        static NoType& test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };

    template <typename T>
    class has_on_close {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C>
        static YesType& test(decltype(&C::on_close));
        template <typename C>
        static NoType& test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };

    template <typename T>
    class has_on_focus_in {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C>
        static YesType& test(decltype(&C::on_focus_in));
        template <typename C>
        static NoType& test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };

    template <typename T>
    class has_on_focus_out {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C>
        static YesType& test(decltype(&C::on_focus_out));
        template <typename C>
        static NoType& test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };

    // Keyboard
    template <typename T>
    class has_on_key_down {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C>
        static YesType& test(decltype(&C::on_key_down));
        template <typename C>
        static NoType& test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };

    template <typename T>
    class has_on_key_up {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C>
        static YesType& test(decltype(&C::on_key_up));
        template <typename C>
        static NoType& test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };

    template <typename T>
    class has_on_char {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C>
        static YesType& test(decltype(&C::on_char));
        template <typename C>
        static NoType& test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };

    // Mouse
    template <typename T>
    class has_on_mouse_button_down {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C>
        static YesType& test(decltype(&C::on_mouse_button_down));
        template <typename C>
        static NoType& test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };

    template <typename T>
    class has_on_mouse_button_up {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C>
        static YesType& test(decltype(&C::on_mouse_button_up));
        template <typename C>
        static NoType& test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };

    template <typename T>
    class has_on_mouse_scroll_v {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C>
        static YesType& test(decltype(&C::on_mouse_scroll_v));
        template <typename C>
        static NoType& test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };

    template <typename T>
    class has_on_mouse_scroll_h {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C>
        static YesType& test(decltype(&C::on_mouse_scroll_h));
        template <typename C>
        static NoType& test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };

    template <typename T>
    class has_on_mouse_move_pos {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C>
        static YesType& test(decltype(&C::on_mouse_move_pos));
        template <typename C>
        static NoType& test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };

    template <typename T>
    class has_on_mouse_move_delta {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C>
        static YesType& test(decltype(&C::on_mouse_move_delta));
        template <typename C>
        static NoType& test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };
} // namespace sw::detail