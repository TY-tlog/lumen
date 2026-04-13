# export/ module — publication-grade export infrastructure (Phase 9).
#
# Provides: ColorProfile, ColorPipeline, FontEmbedder, MathRenderer, ExportTask.
# Depends on: lcms2 (vendored or system), lumen_plot, lumen_data, lumen_core.

include(FindOrFetchLcms2)

add_library(lumen_export STATIC
    ColorProfile.cpp
    ColorProfile.h
    ColorPipeline.cpp
    ColorPipeline.h
    ExportTask.cpp
    ExportTask.h
    FontEmbedder.cpp
    FontEmbedder.h
    MathRenderer.cpp
    MathRenderer.h
)

target_include_directories(lumen_export
    PUBLIC ${CMAKE_SOURCE_DIR}/src/lumen
)

target_link_libraries(lumen_export
    PUBLIC
        Qt6::Core
        Qt6::Gui
        lcms2::lcms2
        lumen_plot
        lumen_io
    PRIVATE
        lumen::compile_options
)
