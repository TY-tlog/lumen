add_library(lumen_dashboard STATIC
    Dashboard.cpp Dashboard.h
    GridLayout.cpp GridLayout.h
    DashboardSerializer.cpp DashboardSerializer.h
    LinkGroup.cpp LinkGroup.h
    LinkChannel.h
    PanelConfig.h
)

# DashboardWidget uses PlotCanvas from lumen_app — compiled there.

target_include_directories(lumen_dashboard
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(lumen_dashboard
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        lumen_plot
    PRIVATE
        lumen::compile_options
)
