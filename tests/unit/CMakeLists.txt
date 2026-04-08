add_executable(lumen_unit_tests
    test_smoke.cpp
    test_dataframe.cpp
    test_csv_reader.cpp
    test_design_tokens.cpp
)

target_compile_definitions(lumen_unit_tests
    PRIVATE
        LUMEN_TEST_FIXTURES_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures"
)

target_link_libraries(lumen_unit_tests
    PRIVATE
        Catch2::Catch2WithMain
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        lumen_data
        lumen::compile_options
        lumen_style
)

catch_discover_tests(lumen_unit_tests)
