# Style module — design tokens, stylesheet manager, palette.

add_library(lumen_style STATIC
    DesignTokens.cpp
    DesignTokens.h
    StyleManager.cpp
    StyleManager.h
    types.h
    cascade.cpp
    cascade.h
    json_io.cpp
    json_io.h
    theme_registry.cpp
    theme_registry.h
)

target_include_directories(lumen_style
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(lumen_style
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        lumen::compile_options
)
