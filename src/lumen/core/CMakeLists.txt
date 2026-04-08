# Lumen core module — event bus, document registry, application state.

add_library(lumen_core STATIC
    DocumentRegistry.cpp
    DocumentRegistry.h
    EventBus.cpp
    EventBus.h
)

target_include_directories(lumen_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(lumen_core
    PUBLIC
        Qt6::Core
        lumen_data
    PRIVATE
        lumen::compile_options
)
