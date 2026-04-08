add_executable(lumen_unit_tests
    test_smoke.cpp
)

target_link_libraries(lumen_unit_tests
    PRIVATE
        Catch2::Catch2WithMain
        Qt6::Core
        Qt6::Widgets
        lumen::compile_options
)

catch_discover_tests(lumen_unit_tests)
