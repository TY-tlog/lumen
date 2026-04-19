# Tests use Catch2 v3 (header + library) fetched via FetchContent.

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.1
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

# Qt needs a platform plugin even for --list-tests (Catch2 discovery).
# Inject QT_QPA_PLATFORM=offscreen into ctest before PRE_TEST discovery runs.
# Also suppress fontconfig's known 320-byte leak (FcInit) so ASan doesn't
# mark every font-touching test as failed.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/set_offscreen.cmake"
    "set(ENV{QT_QPA_PLATFORM} \"offscreen\")\n"
    "set(ENV{LSAN_OPTIONS} \"suppressions=${CMAKE_CURRENT_SOURCE_DIR}/lsan_suppressions.txt\")\n"
)
set_directory_properties(PROPERTIES
    TEST_INCLUDE_FILES "${CMAKE_CURRENT_BINARY_DIR}/set_offscreen.cmake"
)

add_subdirectory(unit)
add_subdirectory(integration)
add_subdirectory(spike)
add_subdirectory(smoke)
