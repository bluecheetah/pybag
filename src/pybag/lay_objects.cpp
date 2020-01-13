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

#include <pybind11_generics/list.h>

#include <cbag/common/box_t.h>
#include <cbag/layout/blockage.h>
#include <cbag/layout/boundary.h>
#include <cbag/layout/cv_obj_ref.h>
#include <cbag/layout/polygons.h>
#include <cbag/layout/via_util.h>
#include <cbag/layout/via_wrapper.h>

#include <pybag/lay_objects.h>

namespace pybag {
namespace layout {

namespace pyg = pybind11_generics;

template <class Shape> decltype(auto) bind_shape_ref(py::module &m, const char *name) {
    auto py_cls = py::class_<cbag::layout::shape_ref<Shape>>(m, name);
    py_cls.def(py::init<>(), "Create an empty layout object.");
    py_cls.def("commit", &cbag::layout::shape_ref<Shape>::commit,
               "Commit this object to cellview.");
    return py_cls;
}

template <class Object> decltype(auto) bind_cv_obj_ref(py::module &m, const char *name) {
    auto py_cls = py::class_<cbag::layout::cv_obj_ref<Object>>(m, name);
    py_cls.def(py::init<>(), "Create an empty layout object.");
    py_cls.def("commit", &cbag::layout::cv_obj_ref<Object>::commit,
               "Commit this object to cellview.");
    return py_cls;
}

cbag::box_t via_bot_box(const cbag::layout::cv_obj_ref<cbag::layout::via_wrapper> &ref) {
    return cbag::layout::get_bot_box(ref->v);
}

cbag::box_t via_top_box(const cbag::layout::cv_obj_ref<cbag::layout::via_wrapper> &ref) {
    return cbag::layout::get_top_box(ref->v);
}

pyg::List<cbag::box_t> via_cuts(const cbag::layout::cv_obj_ref<cbag::layout::via_wrapper> &ref) {
    auto ans = pyg::List<cbag::box_t>();
    cbag::layout::get_via_cuts(ref->v, std::back_inserter(ans));
    return ans;
}

} // namespace layout
} // namespace pybag

void bind_lay_objects(py::module &m) {
    pybag::layout::bind_shape_ref<cbag::box_t>(m, "PyRect");
    pybag::layout::bind_shape_ref<cbag::layout::poly_90_t>(m, "PyPolygon90");
    pybag::layout::bind_shape_ref<cbag::layout::poly_45_t>(m, "PyPolygon45");
    pybag::layout::bind_shape_ref<cbag::layout::poly_t>(m, "PyPolygon");
    pybag::layout::bind_shape_ref<cbag::layout::poly_set_t>(m, "PyPath");

    pybag::layout::bind_cv_obj_ref<cbag::layout::blockage>(m, "PyBlockage");
    pybag::layout::bind_cv_obj_ref<cbag::layout::boundary>(m, "PyBoundary");
    auto py_cls = pybag::layout::bind_cv_obj_ref<cbag::layout::via_wrapper>(m, "PyVia");
    py_cls.def_property_readonly("bottom_box", &pybag::layout::via_bot_box,
                                 "Returns the via bottom bounding box.");
    py_cls.def_property_readonly("top_box", &pybag::layout::via_top_box,
                                 "Returns the via top bounding box.");
    py_cls.def_property_readonly("via_cuts", &pybag::layout::via_cuts,
                                 "Returns a list of via cuts bounding boxes.");
}
