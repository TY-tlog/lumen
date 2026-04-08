add_executable(lumen_unit_tests
    test_smoke.cpp
    test_dataframe.cpp
    test_csv_reader.cpp
    test_design_tokens.cpp
    test_event_bus.cpp
    test_document_registry.cpp
    test_file_loader.cpp
    test_dataframe_table_model.cpp
    ${CMAKE_SOURCE_DIR}/src/lumen/ui/DataFrameTableModel.cpp
    ${CMAKE_SOURCE_DIR}/src/lumen/ui/DataFrameTableModel.h
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
        Qt6::Test
        lumen_data
        lumen_core
        lumen::compile_options
        lumen_style
)

catch_discover_tests(lumen_unit_tests)
