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

if (UNIX AND NOT APPLE)
    find_package(PkgConfig)
    pkg_check_modules(libeditline REQUIRED libedit)
    
    # We want #include <editline/readline.h> not #include <readline.h>
    get_filename_component(libeditline_INCLUDE_DIRS ${libeditline_INCLUDE_DIRS} DIRECTORY)

elseif(WIN32)
    # Hardcoded unfortunately
    set(libeditline_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies-win/wineditline/include")
    set(libeditline_VERSION 2.205)
    find_library(
        libeditline_LIBRARIES
        NAMES edit
        PATHS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies-win/wineditline/bin64"
    )
endif()

mark_as_advanced(
    libeditline_FOUND
    libeditline_INCLUDE_DIRS
    libeditline_LIBRARIES
    libeditline_VERSION
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libeditline
    REQUIRED_VARS libeditline_INCLUDE_DIRS libeditline_LIBRARIES
    VERSION_VAR libeditline_VERSION
)

if (libeditline_FOUND AND NOT TARGET edit)
    add_library(edit INTERFACE IMPORTED)
    set_target_properties(edit PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${libeditline_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${libeditline_LIBRARIES}"
    )
endif()
