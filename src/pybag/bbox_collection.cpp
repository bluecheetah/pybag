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

#include <fmt/core.h>

#include <pybind11_generics/iterator.h>

#include <pybag/bbox_array.h>
#include <pybag/bbox_collection.h>

namespace pyg = pybind11_generics;

namespace pybag {
namespace util {

pyg::PyIterator<c_box_arr> get_box_arr_iter(const c_box_col &bcol) {
    return pyg::make_iterator(bcol.begin(), bcol.end());
}

} // namespace util
} // namespace pybag

namespace pu = pybag::util;

void bind_bbox_collection(py::class_<c_box_col> &py_cls) {

    pyg::declare_iterator<c_box_col::const_iterator>();

    py_cls.doc() = "A collection of BBoxArrays.";
    py_cls.def(py::init<>(), "Create a new BBoxCollection.");

    py_cls.def("__repr__",
               [](const c_box_col &self) -> std::string {
                   switch (self.size()) {
                   case 0:
                       return "BBoxCollection()";
                   case 1:
                       return fmt::format("BBoxCollection({})", pu::to_string(*(self.begin())));
                   default:
                       auto iter = self.begin();
                       auto ans = "BBoxCollection(" + pu::to_string(*iter);
                       ++iter;
                       auto stop = self.end();
                       for (; iter != stop; ++iter) {
                           ans += ", ";
                           ans += pu::to_string(*iter);
                       }
                       ans += ")";
                       return ans;
                   }

               },
               "Returns a string representation of BBoxCollection.");
    py_cls.def("__iter__", &pu::get_box_arr_iter,
               "Returns an iterator over BBoxArray in this collection.");
    py_cls.def("__len__", &c_box_col::size, "Returns the number of BBoxArrays in this collection.");
    py_cls.def("add_rect_arr", &c_box_col::append, "Add bbox to this collection.", py::arg("box"),
               py::arg("nx") = 1, py::arg("ny") = 1, py::arg("spx") = 0, py::arg("spy") = 0);
}
