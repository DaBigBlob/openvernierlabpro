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

if(UNIX AND NOT APPLE)
    find_package(PkgConfig)
    pkg_check_modules(libusb-1.0 REQUIRED libusb-1.0)
    
    # We want #include <libusb-1.0/libusb.h>, not #include <libusb.h>
    get_filename_component(libusb-1.0_INCLUDE_DIRS "${libusb-1.0_INCLUDE_DIRS}" DIRECTORY)
    
elseif(WIN32)
    # Hardcoded unfortunately
    set(libusb-1.0_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies-win/libusb/include")
    set(libusb-1.0_VERSION 1.0.22) # Not necessarily correct, but it's the latest right now.
    find_library(
        libusb-1.0_LIBRARIES
        NAMES libusb-1.0
        PATHS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies-win/libusb/MS64/dll"
    )
endif()

mark_as_advanced(
    libusb-1.0_FOUND
    libusb-1.0_INCLUDE_DIRS
    libusb-1.0_LIBRARIES
    libusb-1.0_VERSION
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libusb-1.0
    REQUIRED_VARS libusb-1.0_INCLUDE_DIRS libusb-1.0_LIBRARIES
    VERSION_VAR libusb-1.0_VERSION
)

if(libusb-1.0_FOUND AND NOT TARGET libusb-1.0)
    add_library(libusb-1.0 INTERFACE IMPORTED)
    set_target_properties(libusb-1.0 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${libusb-1.0_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${libusb-1.0_LIBRARIES}"
    )
endif()
