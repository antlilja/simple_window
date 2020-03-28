add_executable(vulkan_example
	src/window.cpp
	src/vulkan_example.cpp
)

find_package(Vulkan REQUIRED)

target_link_libraries(vulkan_example 
    PRIVATE
        simple::window
		Vulkan::Vulkan
)

target_include_directories(vulkan_example
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(vulkan_example
    PRIVATE
        SW_API_VK
)

target_compile_options(vulkan_example 
    PRIVATE 
        $<$<OR:$<AND:$<CXX_COMPILER_ID:Clang>,$<NOT:$<STREQUAL:"x${CMAKE_CXX_SIMULATE_ID}","xMSVC">>>,$<CXX_COMPILER_ID:GNU>>:
            $<$<CONFIG:Debug>:-O0 -g>
			$<$<CONFIG:Release>:-O3>
        >
        $<$<OR:$<CXX_COMPILER_ID:MSVC>,$<AND:$<STREQUAL:"x${CMAKE_CXX_SIMULATE_ID}","xMSVC">,$<STREQUAL:"${CMAKE_CXX_COMPILER_ID}","Clang">>>:
            $<$<CONFIG:Debug>:/Od /Zi>
		    $<$<CONFIG:Release>:/O2 /Ot /Oi /fp:fast>
		>
)