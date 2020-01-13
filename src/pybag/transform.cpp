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

#include <pybind11/pybind11.h>

#include <pybind11_generics/tuple.h>
#include <pybind11_generics/type_name.h>

#include <cbag/common/transformation_util.h>

#include <pybag/enum_conv.h>
#include <pybag/transform.h>
#include <pybag/typedefs.h>

namespace pyg = pybind11_generics;

using c_transform = cbag::transformation;

void bind_transform(py::module &m) {
    auto py_cls = py::class_<c_transform>(m, "Transform");
    py_cls.doc() = "A class that represents instance transformation.";
    py_cls.def(py::init([](coord_t dx, coord_t dy, int mode) {
                   return c_transform(dx, dy, static_cast<cbag::orientation>(mode));
               }),
               "Create a new transformation object.", py::arg("dx") = 0, py::arg("dy") = 0,
               py::arg("mode") = static_cast<cbag::orient_t>(cbag::orientation::R0));
    py_cls.def("__repr__", [](const c_transform &obj) { return to_string(obj); },
               "Returns a string representation of BBox.");
    py_cls.def_property_readonly("x", [](const c_transform &obj) { return x(obj); }, "X shift.");
    py_cls.def_property_readonly("y", [](const c_transform &obj) { return y(obj); }, "Y shift.");
    py_cls.def_property_readonly("orient",
                                 [](const c_transform &xform) -> pybind11_generics::PyOrient {
                                     return pybag::util::code_to_orient(
                                         static_cast<cbag::orient_t>(xform.orient()));
                                 },
                                 "Orientation.");
    py_cls.def_property_readonly("location",
                                 [](const c_transform &xform) {
                                     auto tmp = xform.offset();
                                     return pyg::Tuple<int, int>::make_tuple(tmp[0], tmp[1]);
                                 },
                                 "Location.");
    py_cls.def_property_readonly(
        "flips_xy", [](const c_transform &xform) { return cbag::swaps_xy(xform.orient()); },
        "True if this transformation flips the axis");
    py_cls.def_property_readonly("axis_scale",
                                 [](const c_transform &xform) {
                                     auto tmp = cbag::axis_scale(xform.orient());
                                     return pyg::Tuple<int, int>::make_tuple(tmp[0], tmp[1]);
                                 },
                                 "The transformation scale factor of each axis.");
    py_cls.def("move_by", &c_transform::move_by, "Moves this transform object", py::arg("dx") = 0,
               py::arg("dy") = 0);
    py_cls.def("get_move_by", &cbag::polygon::get_move_by<cbag::coord_t>,
               "Get a shifted transform object", py::arg("dx") = 0, py::arg("dy") = 0);
    py_cls.def("transform_by",
               [](c_transform &lhs, const c_transform &rhs) {
                   lhs += rhs;
                   return lhs;
               },
               "Transforms this transform object", py::arg("xform"));
    py_cls.def("get_transform_by",
               [](const c_transform &lhs, const c_transform &rhs) {
                   auto ans = lhs;
                   ans += rhs;
                   return ans;
               },
               "Returns a transformed transform object", py::arg("xform"));
    py_cls.def("invert", &c_transform::invert, "Invert this transform object");
    py_cls.def("get_inverse", &cbag::polygon::get_inverse<cbag::coord_t>,
               "Returns an inverted copy of this transform object.");
}
