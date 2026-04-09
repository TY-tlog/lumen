# Plot module — coordinate transforms, data series, scene layout.

add_library(lumen_plot STATIC)

target_sources(lumen_plot PRIVATE
    NiceNumbers.cpp NiceNumbers.h
    CoordinateMapper.cpp CoordinateMapper.h
    ViewTransform.cpp ViewTransform.h
    LineSeries.cpp LineSeries.h
    PlotStyle.h
    Axis.cpp Axis.h
    PlotScene.cpp PlotScene.h
    PlotRenderer.cpp PlotRenderer.h
    HitTester.cpp HitTester.h
)

target_include_directories(lumen_plot
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(lumen_plot
    PUBLIC
        Qt6::Core
        Qt6::Gui
        lumen_data
        lumen_style
    PRIVATE
        lumen::compile_options
)
