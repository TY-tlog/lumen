add_executable(lumen_smoke_tests
    smoke_test_app.cpp
    smoke_test_dashboard.cpp
)

target_include_directories(lumen_smoke_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(lumen_smoke_tests
    PRIVATE
        Catch2::Catch2WithMain
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Test
        lumen_app
)

catch_discover_tests(lumen_smoke_tests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;LSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/../lsan_suppressions.txt"
)
