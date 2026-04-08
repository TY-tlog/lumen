# Lumen data module — CSV parser and data model.

add_library(lumen_data STATIC
    Column.cpp
    Column.h
    ColumnType.h
    CsvError.h
    CsvReader.cpp
    CsvReader.h
    DataFrame.cpp
    DataFrame.h
    FileLoader.cpp
    FileLoader.h
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
