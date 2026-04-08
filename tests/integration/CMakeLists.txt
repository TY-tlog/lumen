add_executable(lumen_integration_tests
    test_open_csv.cpp
)

target_compile_definitions(lumen_integration_tests
    PRIVATE
        LUMEN_TEST_FIXTURES_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures"
)

target_link_libraries(lumen_integration_tests
    PRIVATE
        Catch2::Catch2WithMain
        Qt6::Core
        Qt6::Widgets
        Qt6::Test
        lumen_data
        lumen_core
        lumen::compile_options
)

catch_discover_tests(lumen_integration_tests)
