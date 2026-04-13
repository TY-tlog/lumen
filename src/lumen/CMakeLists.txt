# Lumen executable target.
# Phase 0: minimal main window. Modules added in later phases.

# ---------- OpenGL (optional — ADR-039 GPU layer) ----------
find_package(Qt6 6.4 QUIET OPTIONAL_COMPONENTS OpenGLWidgets)
if(TARGET Qt6::OpenGLWidgets)
    set(LUMEN_HAS_OPENGL_WIDGETS TRUE)
    message(STATUS "Qt6::OpenGLWidgets found — GPU layer enabled")
else()
    set(LUMEN_HAS_OPENGL_WIDGETS FALSE)
    message(STATUS "Qt6::OpenGLWidgets not found — GPU layer disabled (CPU fallback)")
endif()

# ---------- Sub-modules ----------
add_subdirectory(data)
add_subdirectory(style)
add_subdirectory(plot)
if(LUMEN_HAS_OPENGL_WIDGETS)
    add_subdirectory(plot3d)
endif()
add_subdirectory(core)

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
    message(STATUS "Inter font files found -- embedding in binary")
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
    ui/ExportDialog.cpp
    ui/ExportDialog.h
    ui/MainWindow.cpp
    ui/MainWindow.h
    ui/AxisDialog.cpp
    ui/AxisDialog.h
    ui/InteractionController.cpp
    ui/InteractionController.h
    ui/LegendDialog.cpp
    ui/LegendDialog.h
    ui/LinePropertyDialog.cpp
    ui/LinePropertyDialog.h
    ui/ScatterPropertyDialog.cpp
    ui/ScatterPropertyDialog.h
    ui/BarPropertyDialog.cpp
    ui/BarPropertyDialog.h
    ui/HeatmapPropertyDialog.cpp
    ui/HeatmapPropertyDialog.h
    ui/ContourPropertyDialog.cpp
    ui/ContourPropertyDialog.h
    ui/HistogramPropertyDialog.cpp
    ui/HistogramPropertyDialog.h
    ui/BoxPlotPropertyDialog.cpp
    ui/BoxPlotPropertyDialog.h
    ui/ViolinPropertyDialog.cpp
    ui/ViolinPropertyDialog.h
    ui/PlotCanvas.cpp
    ui/PlotCanvas.h
    ui/PlotCanvas3D.cpp
    ui/PlotCanvas3D.h
    ui/PlotCanvas3DDock.cpp
    ui/PlotCanvas3DDock.h
    ui/PlotCanvasDock.cpp
    ui/PlotCanvasDock.h
    ui/ReactivityModeWidget.cpp
    ui/ReactivityModeWidget.h
    ui/TitleDialog.cpp
    ui/TitleDialog.h
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
        lumen_data_io
        lumen_core
        lumen_io
        lumen_reactive
        lumen::compile_options
        lumen_style
        lumen_plot
)

# GPU layer (ADR-039): link Qt6::OpenGLWidgets when available.
if(LUMEN_HAS_OPENGL_WIDGETS)
    target_link_libraries(lumen PRIVATE Qt6::OpenGLWidgets lumen_plot3d)
    target_compile_definitions(lumen PRIVATE LUMEN_HAS_OPENGL_WIDGETS=1)
endif()

set_target_properties(lumen PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_BUNDLE_NAME "Lumen"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.lumen.app"
)
