cmake_minimum_required(VERSION 2.8.7)
project(Radial)
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
add_executable(radial radial.cpp)
target_link_libraries(radial ${VTK_LIBRARIES} ${OPENGL_gl_LIBRARY})
install(TARGETS radial DESTINATION bin)

