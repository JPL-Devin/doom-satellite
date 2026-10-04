# Include project-wide components here

add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/NullPrmDb")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/FrameBufferAdapter")
if (FPRIME_PLATFORM STREQUAL "Zephyr")
    add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/FatalHandler")
endif()
