# Plot module — coordinate transforms, data series, scene layout.

add_library(lumen_plot STATIC)

target_sources(lumen_plot PRIVATE
    NiceNumbers.cpp NiceNumbers.h
    CoordinateMapper.cpp CoordinateMapper.h
    ViewTransform.cpp ViewTransform.h
    PlotItem.h
    LineSeries.cpp LineSeries.h
    ScatterSeries.cpp ScatterSeries.h
    BarSeries.cpp BarSeries.h
    PlotStyle.h
    Axis.cpp Axis.h
    Legend.cpp Legend.h
    PlotScene.cpp PlotScene.h
    PlotRenderer.cpp PlotRenderer.h
    HitTester.cpp HitTester.h
    Colormap.cpp Colormap.h
    Heatmap.cpp Heatmap.h
    ContourAlgorithm.cpp ContourAlgorithm.h
    ContourPlot.cpp ContourPlot.h
    HistogramSeries.cpp HistogramSeries.h
    BoxPlotSeries.cpp BoxPlotSeries.h
    ViolinSeries.cpp ViolinSeries.h
    Annotation.cpp Annotation.h
    AnnotationLayer.cpp AnnotationLayer.h
    annotations/ArrowAnnotation.cpp annotations/ArrowAnnotation.h
    annotations/BoxAnnotation.cpp annotations/BoxAnnotation.h
    annotations/CalloutAnnotation.cpp annotations/CalloutAnnotation.h
    annotations/TextAnnotation.cpp annotations/TextAnnotation.h
    annotations/ScaleBar.cpp annotations/ScaleBar.h
    annotations/ColorBar.cpp annotations/ColorBar.h
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
