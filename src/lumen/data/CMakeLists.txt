# Lumen data module — CSV parser and data model.

add_library(lumen_data STATIC
    Column.cpp
    Column.h
    ColumnType.h
    CoordinateArray.cpp
    CoordinateArray.h
    CsvError.h
    CsvReader.cpp
    CsvReader.h
    DataFrame.cpp
    DataFrame.h
    Dataset.cpp
    Dataset.h
    Dimension.h
    FileLoader.cpp
    FileLoader.h
    Grid2D.cpp
    Grid2D.h
    Rank1Dataset.cpp
    Rank1Dataset.h
    TabularBundle.cpp
    TabularBundle.h
    Unit.cpp
    Unit.h
    Volume3D.cpp
    Volume3D.h
)

target_include_directories(lumen_data
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(lumen_data
    PUBLIC
        Qt6::Core
    PRIVATE
        lumen::compile_options
)
