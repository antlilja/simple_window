add_executable(testing_window
	src/testing_window.cpp
)

target_link_libraries(testing_window 
    PRIVATE
        simple::window
)

target_compile_options(testing_window 
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