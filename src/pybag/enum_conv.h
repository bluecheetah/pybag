// SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
/*
Copyright (c) 2018, Regents of the University of California
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Copyright 2019 Blue Cheetah Analog Design Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef PYBAG_ENUM_CONV_H
#define PYBAG_ENUM_CONV_H

#include <cstdint>

#include <pybind11/pybind11.h>

#include <cbag/common/typedefs.h>
#include <cbag/polygon/enum.h>

namespace py = pybind11;

// custom Orient2D typing definition

namespace pybind11_generics {

using obj_base = py::object;

class PyOrient2D : public obj_base {
  public:
    static bool true_check(PyObject *ptr) { return true; }

    PYBIND11_OBJECT_DEFAULT(PyOrient2D, obj_base, true_check);
};

class PyOrient : public obj_base {
  public:
    static bool true_check(PyObject *ptr) { return true; }

    PYBIND11_OBJECT_DEFAULT(PyOrient, obj_base, true_check);
};

class PyLogLevel : public obj_base {
  public:
    static bool true_check(PyObject *ptr) { return true; }

    PYBIND11_OBJECT_DEFAULT(PyLogLevel, obj_base, true_check);
};

class PySigType : public obj_base {
  public:
    static bool true_check(PyObject *ptr) { return true; }

    PYBIND11_OBJECT_DEFAULT(PySigType, obj_base, true_check);
};

class PyDesignOutput : public obj_base {
  public:
    static bool true_check(PyObject *ptr) { return true; }

    PYBIND11_OBJECT_DEFAULT(PyDesignOutput, obj_base, true_check);
};

} // namespace pybind11_generics

namespace pyg = pybind11_generics;

namespace pybind11 {
namespace detail {

template <> struct handle_type_name<pyg::PyOrient2D> {
    static constexpr auto name = _("pybag.enum.Orient2D");
};

template <> struct handle_type_name<pyg::PyOrient> {
    static constexpr auto name = _("pybag.enum.Orientation");
};

template <> struct handle_type_name<pyg::PyLogLevel> {
    static constexpr auto name = _("pybag.enum.LogLevel");
};

template <> struct handle_type_name<pyg::PySigType> {
    static constexpr auto name = _("pybag.enum.SigType");
};

template <> struct handle_type_name<pyg::PyDesignOutput> {
    static constexpr auto name = _("pybag.enum.DesignOutput");
};

} // namespace detail
} // namespace pybind11

namespace pybag {
namespace util {

cbag::orient_t get_orient_code(const py::str &orient);

pyg::PyOrient code_to_orient(cbag::orient_t code);

pyg::PyOrient2D code_to_orient_2d(cbag::orientation_2d_t code);

pyg::PyLogLevel code_to_log_level(int level);

pyg::PySigType code_to_sig_type(cbag::enum_t sig_type);

} // namespace util
} // namespace pybag

#endif
