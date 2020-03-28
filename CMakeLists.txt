cmake_minimum_required(VERSION 3.2)

project(
	simple_window 
	VERSION 1.0.0
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

# Options
option(SIMPLE_WINDOW_BUILD_EXAMPLES "Builds examples" FALSE)

# Targets
if(UNIX AND NOT APPLE)
	add_library(simple_window 
		src/window_xcb.cpp
	)	
elseif(WIN32)
	add_library(simple_window 
		src/window_win32.cpp
	)	
endif()

add_library(simple::window ALIAS simple_window)

target_include_directories(simple_window PUBLIC include/)

# Libs
if(UNIX AND NOT APPLE)
	find_package(XCB MODULE REQUIRED xcb xcb-cursor)
	target_link_libraries(simple_window PUBLIC ${XCB_LIBRARIES})
endif()


# Definitions
if(WIN32)
	target_compile_definitions(simple_window PUBLIC _UNICODE UNICODE)
endif()

target_compile_features(simple_window PUBLIC cxx_std_17)

# Examples
if(SIMPLE_WINDOW_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()