####
# Zephyr.cmake:
#
# DoomSatellite Zephyr platform: the stock fprime-zephyr platform with sockets enabled, since DoomFlight
# configures the Zephyr network stack (W5500) in prj.conf. Takes precedence over lib/*/cmake/platform/Zephyr.cmake.
####
include("${CMAKE_CURRENT_LIST_DIR}/../../lib/fprime-zephyr/cmake/platform/Zephyr.cmake")
set(FPRIME_HAS_SOCKETS ON)
