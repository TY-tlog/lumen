# Lumen compiler options as INTERFACE library targets.
#
# Usage in module CMakeLists:
#   target_link_libraries(my_target PRIVATE lumen::compile_options)

add_library(lumen_compile_options INTERFACE)
add_library(lumen::compile_options ALIAS lumen_compile_options)

if(LUMEN_ENABLE_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(lumen_compile_options INTERFACE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wconversion
            -Wsign-conversion
            # -Wnull-dereference disabled: GCC -O3 produces false positives
            # with shared_ptr inline member access (e.g. scatter->xDataset()->...)
            # Re-enable when GCC fixes this or when we add explicit asserts everywhere.
            -Wdouble-promotion
            -Wformat=2
            -Wimplicit-fallthrough
        )
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            target_compile_options(lumen_compile_options INTERFACE
                -Wmisleading-indentation
                -Wduplicated-cond
                -Wduplicated-branches
                -Wlogical-op
                -Wuseless-cast
            )
        endif()
    endif()

    if(LUMEN_WARNINGS_AS_ERRORS)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
            target_compile_options(lumen_compile_options INTERFACE -Werror)
        endif()
    endif()
endif()

# Sanitizers (Debug only)
if(LUMEN_ENABLE_ASAN AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(lumen_compile_options INTERFACE
        -fsanitize=address
        -fno-omit-frame-pointer
    )
    target_link_options(lumen_compile_options INTERFACE -fsanitize=address)
    message(STATUS "AddressSanitizer enabled")
endif()

if(LUMEN_ENABLE_UBSAN AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(lumen_compile_options INTERFACE
        -fsanitize=undefined
        -fno-omit-frame-pointer
    )
    target_link_options(lumen_compile_options INTERFACE -fsanitize=undefined)
    message(STATUS "UndefinedBehaviorSanitizer enabled")
endif()
