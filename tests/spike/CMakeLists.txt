add_executable(lumen_spike_tests
    test_grid2d_arithmetic.cpp
)

target_include_directories(lumen_spike_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/lumen
)

target_link_libraries(lumen_spike_tests
    PRIVATE
        Catch2::Catch2WithMain
        Qt6::Core
        lumen_data
        lumen::compile_options
)

catch_discover_tests(lumen_spike_tests)
