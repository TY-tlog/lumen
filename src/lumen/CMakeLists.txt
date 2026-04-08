# Lumen executable target.
# Phase 0: minimal main window. Modules added in later phases.

qt_add_executable(lumen
    main.cpp
    app/Application.cpp
    app/Application.h
    ui/MainWindow.cpp
    ui/MainWindow.h
)

target_include_directories(lumen
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(lumen
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        lumen::compile_options
)

set_target_properties(lumen PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_BUNDLE_NAME "Lumen"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.lumen.app"
)

# Resources (QSS, icons, fonts) — added when files exist
# qt_add_resources(lumen "app_resources" PREFIX "/" FILES ...)
