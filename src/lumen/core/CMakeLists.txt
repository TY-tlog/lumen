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
)

target_include_directories(lumen_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(lumen_core
    PUBLIC
        Qt6::Core
        lumen_data
        lumen_plot
    PRIVATE
        lumen::compile_options
)

add_subdirectory(io)
add_subdirectory(reactive)
