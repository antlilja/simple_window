#pragma once

#if defined(_WIN32)
#    include "simple_window/window_win32_interface.hpp"
#elif defined(__linux__)
#    include "simple_window/window_xcb_interface.hpp"
#endif
