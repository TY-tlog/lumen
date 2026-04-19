# Lumen core module — event bus, document registry, application state.

add_library(lumen_core STATIC
    Command.h
    CommandBus.cpp
    CommandBus.h
    DocumentRegistry.cpp
    DocumentRegistry.h
    EventBus.cpp
    EventBus.h
    PlotRegistry.cpp
    PlotRegistry.h
    commands/ChangeAxisPropertiesCommand.cpp
    commands/ChangeAxisPropertiesCommand.h
    commands/ChangeLegendCommand.cpp
    commands/ChangeLegendCommand.h
    commands/ChangeLineStyleCommand.cpp
    commands/ChangeLineStyleCommand.h
    commands/ChangeTitleCommand.cpp
    commands/ChangeTitleCommand.h
    commands/ChangeScatterPropertiesCommand.cpp
    commands/ChangeScatterPropertiesCommand.h
    commands/ChangeBarPropertiesCommand.cpp
    commands/ChangeBarPropertiesCommand.h
    commands/ChangeHeatmapPropertiesCommand.cpp
    commands/ChangeHeatmapPropertiesCommand.h
    commands/ChangeContourPropertiesCommand.cpp
    commands/ChangeContourPropertiesCommand.h
    commands/ChangeHistogramPropertiesCommand.cpp
    commands/ChangeHistogramPropertiesCommand.h
    commands/ChangeBoxPlotPropertiesCommand.cpp
    commands/ChangeBoxPlotPropertiesCommand.h
    commands/ChangeViolinPropertiesCommand.cpp
    commands/ChangeViolinPropertiesCommand.h
    commands/ChangeAnnotationCommand.cpp
    commands/ChangeAnnotationCommand.h
    commands/StyleEditCommand.cpp
    commands/StyleEditCommand.h
    commands/DashboardCommands.cpp
    commands/DashboardCommands.h
)

target_include_directories(lumen_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(lumen_core
    PUBLIC
        Qt6::Core
        lumen_data
        lumen_dashboard
        lumen_plot
    PRIVATE
        lumen::compile_options
)

# 3D commands depend on lumen_plot3d (requires OpenGL).
if(TARGET lumen_plot3d)
    target_sources(lumen_core PRIVATE
        commands/ChangeScatter3DPropertiesCommand.cpp
        commands/ChangeScatter3DPropertiesCommand.h
    )
    target_link_libraries(lumen_core PUBLIC lumen_plot3d)
    target_compile_definitions(lumen_core PUBLIC LUMEN_HAS_OPENGL_WIDGETS=1)
endif()

add_subdirectory(io)
add_subdirectory(reactive)
