# CMake Script for liblabpro (liblabpro.sf.net)
# Based on modern-cmake-sample by Pablo Arias (https://github.com/pabloariasal/modern-cmake-sample)
# 
# Copyright (C) 2018 Pablo Arias <pabloariasal@gmail.com>
# Copyright (C) 2018 Matthew Trescott <matthewtrescott@gmail.com>
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the Software
# is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
# PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
# CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# 
# And the CMAKE_MODULE_PATH line is from SuperTuxKart's CMakeLists.txt (GPLv3) but I
# don't think that qualifies for copyright coverage.

cmake_minimum_required(VERSION 3.9)
project(liblabpro
    VERSION 1.0
    DESCRIPTION "A library to access Vernier LabPro interfaces"
    LANGUAGES C
)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeScripts")

add_library(labpro SHARED src/liblabpro.c)
target_include_directories(labpro
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include/liblabpro>
)

find_package(libusb-1.0 REQUIRED)
target_link_libraries(labpro
    PUBLIC
        libusb-1.0
)

if(UNIX AND NOT APPLE)
    # I wish I had known earlier that this check had to be enabled manually for shared libraries
    set_target_properties(labpro PROPERTIES LINK_FLAGS " -Wl,--no-undefined")
endif()

# This makes Visual Studio generate a .lib file so that other programs can link to the DLL.
if(WIN32)
    set_target_properties(labpro PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)
endif()

if(UNIX AND NOT APPLE)
    include(GNUInstallDirs)
    # Install header
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/src/liblabpro.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/liblabpro/)
    # Install .so
    install(TARGETS labpro
        EXPORT liblabpro-export
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    # Install CMake scripts
    install(EXPORT liblabpro-export
        FILE liblabpro-targets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/liblabpro
    )
    # Install pkg-config file
    configure_file(${CMAKE_CURRENT_LIST_DIR}/liblabpro.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/liblabpro.pc
        @ONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/liblabpro.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig/)
endif()

include(CMakePackageConfigHelpers)
# Generate Config-mode package version file for find_package
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/liblabproConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

if (UNIX AND NOT APPLE)
    # Generate Config-mode package file for find_package
    configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/CMakeScripts/liblabproConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/liblabproConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/liblabpro
    )

    # Install the CMake scripts for other programs to use
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/liblabproConfigVersion.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/liblabproConfig.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/liblabpro
    )

    export(
        EXPORT liblabpro-export
        FILE ${CMAKE_CURRENT_BINARY_DIR}/liblabpro-targets.cmake
    )
endif()

# Console
add_executable(labpro-console src/console.c)
find_package(libeditline)
target_link_libraries(labpro-console PRIVATE labpro edit)
