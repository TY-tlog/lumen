# Plot3D module — 3D rendering infrastructure (Phase 8).

add_library(lumen_plot3d STATIC
    Camera.cpp Camera.h
    Light.h
    Scene3D.cpp Scene3D.h
    PlotItem3D.h
    BoundingBox3D.h
    PbrMaterial.h
    Ray.cpp Ray.h
    RenderContext.h
    Renderer3D.cpp Renderer3D.h
    Scatter3D.cpp Scatter3D.h
    Surface3D.cpp Surface3D.h
    TransferFunction.cpp TransferFunction.h
    VolumeItem.cpp VolumeItem.h
    Streamlines.cpp Streamlines.h
    MarchingCubes.cpp MarchingCubes.h
    Isosurface.cpp Isosurface.h
    ShaderProgram.cpp ShaderProgram.h
    SpatialGrid3D.cpp SpatialGrid3D.h
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
        lumen_plot
        lumen::compile_options
)

# Suppress -Wdouble-promotion for plot3d: OpenGL math uses float extensively,
# and std::sin/cos/sqrt overload resolution differs between GCC and Clang.
# Must come AFTER linking lumen::compile_options which adds -Wdouble-promotion.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(lumen_plot3d PRIVATE -Wno-error=double-promotion)
endif()
