CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(ECX)

set(ICET_SRCDIR   ${CMAKE_SOURCE_DIR}/pvol-icet)
set(ICET_OBJDIR   ${CMAKE_BINARY_DIR}/pvol-icet)
set(OSPRAY_SRCDIR ${CMAKE_SOURCE_DIR}/pvol-ospray)
set(OSPRAY_OBJDIR ${CMAKE_BINARY_DIR}/pvol-ospray)

add_subdirectory(pvol-icet)
add_subdirectory(pvol-ospray)
add_subdirectory(pvol)
add_subdirectory(SimSim)
add_subdirectory(radial)

install(PROGRAMS ecx.env DESTINATION ${CMAKE_INSTALL_PREFIX})

