# SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
# Copyright 2018 Regents of the University of California
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Copyright 2019 Blue Cheetah Analog Design Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Version 3.8 required because of C++17 support.
cmake_minimum_required(VERSION 3.8)
project(pybag)

# Boost Spirit X3 requires C++14
# __has_include directive requires C++17
# std::variant requires C++17
# map::extract (use of node handles) requires C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# control compiler warnings
add_compile_options(-fexceptions)
add_compile_options(-fmax-errors=2)
add_compile_options(-g)
add_compile_options(-Wall)
add_compile_options(-Wno-delete-non-virtual-dtor)
# add_compile_options(-Wno-logical-op-parentheses)
# add_compile_options(-Wno-new-returns-null)
# set optimzation level for release
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
# generate compilation commands file for emacs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Set to build position independent code
# Since Python wrappers are always shared library,
# We need to build static library dependencies with
# position independent code.
set(CMAKE_POSITION_INDEPENDENT_CODE True)
# prefer pthreads
set(THREADS_PREFER_PTHREAD_FLAG ON)

# make sure linker raise errors if shared library has undefined symbols
# this makes it a lot easier to debug
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
# add rpaths to the final install executable
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

# add cbag submodule
add_subdirectory(cbag)

# add pybind11_generics submodule
add_subdirectory(pybind11_generics)

# Include yaml-cpp
find_package(yaml-cpp REQUIRED CONFIG)

# add python bindings for cbag
pybind11_add_module(core
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/bbox.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/bbox_array.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/bbox_collection.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/core.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/enum_conv.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/gds.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/geometry.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/grid.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/interval.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/lay_objects.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/layout.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/logging.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/name.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/oa.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/rtree.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/schematic.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/tech.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybag/transform.cpp
  )

target_include_directories(core
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

target_link_libraries(core
  PUBLIC
  cbag
  PRIVATE
  pybind11_generics
  yaml-cpp
  )

if( DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY )
  set_target_properties(core
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    )
else()
  message( FATAL_ERROR "CMAKE_LIBRARY_OUTPUT_DIRECTORY not defined!" )
endif()
