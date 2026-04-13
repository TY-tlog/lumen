# Plot3D module — 3D rendering infrastructure (Phase 8).

add_library(lumen_plot3d STATIC
    Camera.cpp Camera.h
    Light.h
    Scene3D.cpp Scene3D.h
    PlotItem3D.h
    BoundingBox3D.h
    Ray.cpp Ray.h
    RenderContext.h
    Renderer3D.cpp Renderer3D.h
    ShaderProgram.cpp ShaderProgram.h
)

target_include_directories(lumen_plot3d
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(lumen_plot3d
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::OpenGLWidgets
    PRIVATE
        lumen_data
        lumen::compile_options
)
