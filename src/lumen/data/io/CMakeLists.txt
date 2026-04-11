# Lumen data I/O module — dataset loaders and registry.

set(IO_SOURCES
    DatasetLoader.cpp
    DatasetLoader.h
    LoaderRegistry.cpp
    LoaderRegistry.h
    CsvLoader.cpp
    CsvLoader.h
    JsonLoader.cpp
    JsonLoader.h
    NumpyLoader.cpp
    NumpyLoader.h
)

set(IO_CONDITIONAL_SOURCES "")
set(IO_CONDITIONAL_LIBS "")
set(IO_CONDITIONAL_DEFS "")

# ---------- HDF5 ----------
find_package(HDF5 COMPONENTS C)
if(HDF5_FOUND)
    message(STATUS "HDF5 found: ${HDF5_VERSION} -> enabling Hdf5Loader")
    list(APPEND IO_CONDITIONAL_SOURCES Hdf5Loader.cpp Hdf5Loader.h)
    list(APPEND IO_CONDITIONAL_LIBS ${HDF5_C_LIBRARIES})
    list(APPEND IO_CONDITIONAL_DEFS LUMEN_HAS_HDF5)
else()
    message(STATUS "HDF5 not found -- Hdf5Loader disabled")
endif()

# ---------- NetCDF ----------
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(NETCDF QUIET netcdf)
endif()
if(NOT NETCDF_FOUND)
    # Try manual detection
    find_library(NETCDF_LIBRARIES NAMES netcdf)
    find_path(NETCDF_INCLUDE_DIRS NAMES netcdf.h)
    if(NETCDF_LIBRARIES AND NETCDF_INCLUDE_DIRS)
        set(NETCDF_FOUND TRUE)
    endif()
endif()
if(NETCDF_FOUND)
    message(STATUS "NetCDF found -> enabling NetCDFLoader")
    list(APPEND IO_CONDITIONAL_SOURCES NetCDFLoader.cpp NetCDFLoader.h)
    list(APPEND IO_CONDITIONAL_LIBS ${NETCDF_LIBRARIES})
    list(APPEND IO_CONDITIONAL_DEFS LUMEN_HAS_NETCDF)
else()
    message(STATUS "NetCDF not found -- NetCDFLoader disabled")
endif()

# ---------- TIFF ----------
find_package(TIFF QUIET)
if(TIFF_FOUND)
    message(STATUS "TIFF found -> enabling TiffStackLoader")
    list(APPEND IO_CONDITIONAL_SOURCES TiffStackLoader.cpp TiffStackLoader.h)
    list(APPEND IO_CONDITIONAL_LIBS TIFF::TIFF)
    list(APPEND IO_CONDITIONAL_DEFS LUMEN_HAS_TIFF)
else()
    message(STATUS "TIFF not found -- TiffStackLoader disabled")
endif()

# ---------- MATIO ----------
if(PkgConfig_FOUND)
    pkg_check_modules(MATIO QUIET matio)
endif()
if(NOT MATIO_FOUND)
    find_library(MATIO_LIBRARIES NAMES matio)
    find_path(MATIO_INCLUDE_DIRS NAMES matio.h)
    if(MATIO_LIBRARIES AND MATIO_INCLUDE_DIRS)
        set(MATIO_FOUND TRUE)
    endif()
endif()
if(MATIO_FOUND)
    message(STATUS "MATIO found -> enabling MatLoader")
    list(APPEND IO_CONDITIONAL_SOURCES MatLoader.cpp MatLoader.h)
    list(APPEND IO_CONDITIONAL_LIBS ${MATIO_LIBRARIES})
    list(APPEND IO_CONDITIONAL_DEFS LUMEN_HAS_MATIO)
else()
    message(STATUS "MATIO not found -- MatLoader disabled")
endif()

# ---------- Build library ----------
add_library(lumen_data_io STATIC
    ${IO_SOURCES}
    ${IO_CONDITIONAL_SOURCES}
)

target_include_directories(lumen_data_io
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

if(HDF5_FOUND)
    target_include_directories(lumen_data_io PRIVATE ${HDF5_INCLUDE_DIRS})
endif()
if(NETCDF_FOUND)
    target_include_directories(lumen_data_io PRIVATE ${NETCDF_INCLUDE_DIRS})
endif()
if(MATIO_FOUND)
    target_include_directories(lumen_data_io PRIVATE ${MATIO_INCLUDE_DIRS})
endif()

target_compile_definitions(lumen_data_io
    PUBLIC
        ${IO_CONDITIONAL_DEFS}
)

target_link_libraries(lumen_data_io
    PUBLIC
        Qt6::Core
        lumen_data
    PRIVATE
        lumen::compile_options
        ${IO_CONDITIONAL_LIBS}
)
