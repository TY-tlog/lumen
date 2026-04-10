# Lumen I/O module — workspace sidecar files and workspace management.

add_library(lumen_io STATIC
    FigureExporter.cpp
    FigureExporter.h
    WorkspaceFile.cpp
    WorkspaceFile.h
    WorkspaceManager.cpp
    WorkspaceManager.h
)

target_include_directories(lumen_io
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(lumen_io
    PUBLIC
        Qt6::Core
        lumen_data
        lumen_plot
        lumen_core
    PRIVATE
        lumen::compile_options
)
