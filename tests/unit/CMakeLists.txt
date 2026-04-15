add_executable(lumen_unit_tests
    test_smoke.cpp
    test_dataframe.cpp
    test_csv_reader.cpp
    test_design_tokens.cpp
    test_event_bus.cpp
    test_document_registry.cpp
    test_file_loader.cpp
    test_dataframe_table_model.cpp
    test_nice_numbers.cpp
    test_coordinate_mapper.cpp
    test_view_transform.cpp
    test_line_series.cpp
    test_scatter_series.cpp
    test_bar_series.cpp
    test_axis.cpp
    test_axis_setters.cpp
    test_plot_scene.cpp
    test_plot_scene_title.cpp
    test_plot_renderer.cpp
    test_plot_registry.cpp
    test_command_bus.cpp
    test_hit_tester.cpp
    test_legend.cpp
    test_hit_tester_regions.cpp
    test_plot_scene_margins.cpp
    test_axis_commands.cpp
    test_title_command.cpp
    test_legend_commands.cpp
    test_scatter_commands.cpp
    test_bar_commands.cpp
    test_hit_test_point.cpp
    test_workspace_file.cpp
    test_workspace_manager.cpp
    test_figure_exporter.cpp
    test_unit_parsing.cpp
    test_unit_arithmetic.cpp
    test_unit_conversion.cpp
    test_dimension.cpp
    test_coordinate_array.cpp
    test_dataset_base.cpp
    test_rank1_dataset.cpp
    test_tabular_bundle.cpp
    test_grid2d.cpp
    test_volume3d.cpp
    test_csv_loader.cpp
    test_json_loader.cpp
    test_numpy_loader.cpp
    test_memory_manager.cpp
    test_loader_registry.cpp
    test_hdf5_loader.cpp
    test_netcdf_loader.cpp
    test_data_table_dock_placeholders.cpp
    test_reactive_mode.cpp
    test_dependency_graph.cpp
    test_reactive_binding.cpp
    test_colormap_builtin.cpp
    test_colormap_perceptual.cpp
    test_colormap_colorblind.cpp
    test_colormap_json_roundtrip.cpp
    test_colormap_interpolation.cpp
    test_colormap_edge_cases.cpp
    test_heatmap.cpp
    test_heatmap_cache.cpp
    test_heatmap_gpu_fallback.cpp
    test_contour_algorithm.cpp
    test_contour_plot.cpp
    test_histogram.cpp
    test_boxplot.cpp
    test_violin.cpp
    test_camera_trackball.cpp
    test_camera_orbit.cpp
    test_camera_json.cpp
    test_ray_unprojection.cpp
    test_scene3d.cpp
    test_bounding_box3d.cpp
    test_shader_program.cpp
    test_renderer3d.cpp
    test_scatter3d.cpp
    test_spatial_grid3d.cpp
    test_plotcanvas3d.cpp
    test_scatter3d_command.cpp
    test_surface3d.cpp
    test_transfer_function.cpp
    test_volume_item.cpp
    test_streamlines.cpp
    test_isosurface.cpp
    test_pbr_material.cpp
    test_histogram_commands.cpp
    test_boxplot_commands.cpp
    test_violin_commands.cpp
    test_heatmap_commands.cpp
    test_contour_commands.cpp
    test_camera_mode_switch.cpp
    test_plotcanvas3d_dock.cpp
    test_3d_edge_cases.cpp
    test_color_profile_builtin.cpp
    test_color_profile_from_file.cpp
    test_color_conversion.cpp
    test_png_iccp_chunk.cpp
    test_pdf_iccbased_colorspace.cpp
    test_annotation_layer.cpp
    test_arrow_annotation.cpp
    test_text_annotation.cpp
    test_annotation_workspace.cpp
    test_font_embedder_subset.cpp
    test_academic_fonts_loaded.cpp
    test_microtex_basic.cpp
    test_microtex_complex.cpp
    test_microtex_invalid.cpp
    test_export_task_cancel.cpp
    test_text_as_path.cpp
    test_microtex_tier1.cpp
    test_cascade.cpp
    test_schema_validation.cpp
    test_json_roundtrip.cpp
    test_theme_registry.cpp
    test_theme_fork.cpp
    test_extends_resolver.cpp
    test_clipboard.cpp
    ${CMAKE_SOURCE_DIR}/src/lumen/ui/DataFrameTableModel.cpp
    ${CMAKE_SOURCE_DIR}/src/lumen/ui/PlotCanvas3D.cpp
    ${CMAKE_SOURCE_DIR}/src/lumen/ui/PlotCanvas3D.h
    ${CMAKE_SOURCE_DIR}/src/lumen/ui/PlotCanvas3DDock.cpp
    ${CMAKE_SOURCE_DIR}/src/lumen/ui/PlotCanvas3DDock.h
)

target_compile_definitions(lumen_unit_tests
    PRIVATE
        LUMEN_TEST_FIXTURES_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures"
)

# I/O loader tests need library include paths for creating test fixtures
find_package(HDF5 COMPONENTS C QUIET)
if(HDF5_FOUND)
    target_include_directories(lumen_unit_tests PRIVATE ${HDF5_INCLUDE_DIRS})
    target_link_libraries(lumen_unit_tests PRIVATE ${HDF5_C_LIBRARIES})
endif()

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(NETCDF QUIET netcdf)
endif()
if(NOT NETCDF_FOUND)
    find_library(NETCDF_LIBRARIES NAMES netcdf)
    find_path(NETCDF_INCLUDE_DIRS NAMES netcdf.h)
    if(NETCDF_LIBRARIES AND NETCDF_INCLUDE_DIRS)
        set(NETCDF_FOUND TRUE)
    endif()
endif()
if(NETCDF_FOUND)
    target_include_directories(lumen_unit_tests PRIVATE ${NETCDF_INCLUDE_DIRS})
    target_link_libraries(lumen_unit_tests PRIVATE ${NETCDF_LIBRARIES})
endif()

target_link_libraries(lumen_unit_tests
    PRIVATE
        Catch2::Catch2WithMain
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Test
        lumen_data
        lumen_data_io
        lumen_core
        lumen_io
        lumen_reactive
        lumen::compile_options
        lumen_style
        lumen_plot
        lumen_export
)

# Link lumen_plot3d when OpenGL widgets are available.
if(TARGET lumen_plot3d)
    target_link_libraries(lumen_unit_tests PRIVATE lumen_plot3d)
    target_compile_definitions(lumen_unit_tests PRIVATE LUMEN_HAS_OPENGL_WIDGETS=1)
endif()

catch_discover_tests(lumen_unit_tests)
