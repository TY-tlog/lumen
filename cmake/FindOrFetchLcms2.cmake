# Find system lcms2, or fetch via FetchContent as fallback.
#
# Defines target: lcms2::lcms2

include(FetchContent)

# Try to find the library directly (works on both Linux and macOS/Homebrew).
find_path(LCMS2_INCLUDE_DIR NAMES lcms2.h
    HINTS /opt/homebrew/include /usr/local/include)
find_library(LCMS2_LIBRARY NAMES lcms2
    HINTS /opt/homebrew/lib /usr/local/lib)

if(LCMS2_INCLUDE_DIR AND LCMS2_LIBRARY)
    set(LCMS2_FOUND TRUE)
    message(STATUS "lcms2 found (system): ${LCMS2_LIBRARY}")
    if(NOT TARGET lcms2::lcms2)
        add_library(lcms2::lcms2 UNKNOWN IMPORTED)
        set_target_properties(lcms2::lcms2 PROPERTIES
            IMPORTED_LOCATION "${LCMS2_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${LCMS2_INCLUDE_DIR}"
        )
    endif()
else()
    message(STATUS "lcms2 not found — fetching via FetchContent")
    FetchContent_Declare(
        lcms2
        GIT_REPOSITORY https://github.com/mm2/Little-CMS.git
        GIT_TAG        lcms2.16
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(lcms2)

    # Little-CMS builds a 'lcms2' target. Create alias.
    if(TARGET lcms2 AND NOT TARGET lcms2::lcms2)
        add_library(lcms2::lcms2 ALIAS lcms2)
    endif()
endif()
