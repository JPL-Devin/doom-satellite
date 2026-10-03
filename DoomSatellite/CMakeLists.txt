# This CMake file is intended to register project-wide objects.
# This allows for reuse between deployments, or other projects.

add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/project/config")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Components")

# Zephyr supports one deployment per build, so each deployment is selected by its platform
if (FPRIME_PLATFORM STREQUAL "Zephyr")
    add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/DoomFlight/")
else()
    add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/DoomCoprocessor/")
endif()
