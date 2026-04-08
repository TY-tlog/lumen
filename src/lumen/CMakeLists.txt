# Lumen executable target.
# Phase 0: minimal main window. Modules added in later phases.

# ---------- Sub-modules ----------
add_subdirectory(data)
add_subdirectory(style)

# ---------- Embedded resources: fonts (conditional) ----------
set(_font_dir "${CMAKE_SOURCE_DIR}/resources/fonts")
set(_fonts_present TRUE)
foreach(_f Inter-Regular.ttf Inter-Medium.ttf Inter-SemiBold.ttf Inter-Bold.ttf)
    if(NOT EXISTS "${_font_dir}/${_f}")
        set(_fonts_present FALSE)
        break()
    endif()
endforeach()

if(_fonts_present)
    message(STATUS "Inter font files found — embedding in binary")
    set(LUMEN_FONT_QRC "${_font_dir}/fonts.qrc")
else()
    message(WARNING "Inter font files not found in resources/fonts/. "
        "Run: bash scripts/download_inter_font.sh")
    set(LUMEN_FONT_QRC "")
endif()

# ---------- Main executable ----------
qt_add_executable(lumen
    main.cpp
    app/Application.cpp
    app/Application.h
    ui/DataFrameTableModel.cpp
    ui/DataFrameTableModel.h
    ui/DataTableDock.cpp
    ui/DataTableDock.h
    ui/MainWindow.cpp
    ui/MainWindow.h
    "${CMAKE_SOURCE_DIR}/resources/styles/styles.qrc"
    ${LUMEN_FONT_QRC}
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
        lumen_data
        lumen::compile_options
        lumen_style
)

set_target_properties(lumen PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_BUNDLE_NAME "Lumen"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.lumen.app"
)
