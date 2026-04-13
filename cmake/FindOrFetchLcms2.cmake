# Find system lcms2, or fetch via FetchContent as fallback.
#
# Sets: LCMS2_FOUND, LCMS2_INCLUDE_DIRS, LCMS2_LIBRARIES
# Or defines a lcms2 target via FetchContent.

include(FetchContent)

# Try system package first (faster, no build overhead).
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(LCMS2 QUIET lcms2)
endif()

if(NOT LCMS2_FOUND)
    find_path(LCMS2_INCLUDE_DIRS NAMES lcms2.h)
    find_library(LCMS2_LIBRARIES NAMES lcms2)
    if(LCMS2_INCLUDE_DIRS AND LCMS2_LIBRARIES)
        set(LCMS2_FOUND TRUE)
    endif()
endif()

if(LCMS2_FOUND)
    message(STATUS "lcms2 found (system): ${LCMS2_LIBRARIES}")
    # Create an imported target for uniform usage.
    if(NOT TARGET lcms2::lcms2)
        add_library(lcms2::lcms2 INTERFACE IMPORTED)
        set_target_properties(lcms2::lcms2 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${LCMS2_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${LCMS2_LIBRARIES}"
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
