# Reactive core module -- DependencyGraph, ReactiveBinding, three-mode reactivity (ADR-038).

add_library(lumen_reactive STATIC
    ReactiveMode.h
    DependencyGraph.cpp
    DependencyGraph.h
    ReactiveBinding.cpp
    ReactiveBinding.h
)

target_include_directories(lumen_reactive
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(lumen_reactive
    PUBLIC
        Qt6::Core
        lumen_data
        lumen_plot
    PRIVATE
        lumen::compile_options
)
