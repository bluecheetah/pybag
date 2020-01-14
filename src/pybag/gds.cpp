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

#include <memory>

#include <pybind11/pybind11.h>

#include <pybind11_generics/iterable.h>
#include <pybind11_generics/list.h>

#include <cbag/gdsii/main.h>
#include <cbag/gdsii/read.h>
#include <cbag/gdsii/write.h>
#include <cbag/layout/cellview.h>
#include <cbag/layout/routing_grid_fwd.h>

#include <pybag/gds.h>

namespace py = pybind11;
namespace pyg = pybind11_generics;

using c_lay_cv = cbag::layout::cellview;
using c_lay_cv_info = std::pair<std::string, c_lay_cv *>;
using py_cv_list = pyg::List<std::shared_ptr<c_lay_cv>>;

namespace pybag {
namespace util {

py_cv_list read_gds(const std::string &fname, const std::string &layer_map,
                    const std::string &obj_map,
                    const std::shared_ptr<cbag::layout::routing_grid> &grid_ptr,
                    const std::shared_ptr<cbag::layout::track_coloring> &tr_colors) {
    py_cv_list ans;
    cbag::gdsii::read_gds(fname, layer_map, obj_map, grid_ptr, tr_colors, std::back_inserter(ans));
    return ans;
}

} // namespace util
} // namespace pybag

void bind_gds(py::module &m) {
    m.def("implement_gds", &cbag::gdsii::implement_gds<pyg::Iterable<c_lay_cv_info>>,
          "Write the given layouts to GDS.", py::arg("fname"), py::arg("lib_name"),
          py::arg("layer_map"), py::arg("obj_map"), py::arg("cv_list"));

    m.def("read_gds", &pybag::util::read_gds, "Reads layout cellviews from the given GDS file.",
          py::arg("fname"), py::arg("layer_map"), py::arg("obj_map"), py::arg("grid"),
          py::arg("tr_colors"));

    m.def("gds_equal", &cbag::gdsii::gds_equal, "Returns True if both gds files are equivalent.",
          py::arg("lhs_file"), py::arg("rhs_file"));
}
