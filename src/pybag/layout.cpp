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
#include <pybind11/stl.h>

#include <cbag/common/box_collection.h>
#include <cbag/common/transformation_util.h>
#include <cbag/layout/cellview_poly.h>
#include <cbag/layout/cellview_util.h>
#include <cbag/layout/grid_object.h>
#include <cbag/layout/instance.h>
#include <cbag/layout/path_util.h>
#include <cbag/layout/via_wrapper.h>

#include <pybag/bbox_array.h>
#include <pybag/layout.h>
#include <pybag/py_pt_vector.h>

namespace pyg = pybind11_generics;

using c_instance = cbag::layout::instance;
using c_inst_ref = cbag::layout::cv_obj_ref<c_instance>;
using c_cellview = cbag::layout::cellview;

namespace pybag {
namespace lay {

void check_ref(const c_inst_ref &ref) {
    if (!ref.editable()) {
        throw std::runtime_error("Cannot modify committed instance");
    }
}

cbag::cnt_t get_nx(const c_inst_ref &ref) { return ref->nx; }
cbag::cnt_t get_ny(const c_inst_ref &ref) { return ref->ny; }
cbag::offset_t get_spx(const c_inst_ref &ref) { return ref->spx; }
cbag::offset_t get_spy(const c_inst_ref &ref) { return ref->spy; }
const cbag::transformation &get_xform(const c_inst_ref &ref) { return ref->xform; }

void set_nx(c_inst_ref &ref, cbag::scnt_t val) {
    check_ref(ref);
    if (val < 0)
        throw std::runtime_error("Cannot set nx to be negative.");
    ref->nx = val;
}
void set_ny(c_inst_ref &ref, cbag::scnt_t val) {
    check_ref(ref);
    if (val < 0)
        throw std::runtime_error("Cannot set nx to be negative.");
    ref->ny = val;
}
void set_spx(c_inst_ref &ref, cbag::offset_t val) {
    check_ref(ref);
    ref->spx = val;
}
void set_spy(c_inst_ref &ref, cbag::offset_t val) {
    check_ref(ref);
    ref->spy = val;
}

void move_by(c_inst_ref &ref, cbag::offset_t dx, cbag::offset_t dy) {
    check_ref(ref);
    ref->xform.move_by(dx, dy);
}

void transform(c_inst_ref &ref, const cbag::transformation &xform) {
    check_ref(ref);
    ref->xform += xform;
}

void set_master(c_inst_ref &ref, const std::shared_ptr<const cbag::layout::cellview> &new_master) {
    check_ref(ref);
    ref->set_master(new_master);
}

pyg::List<cbag::box_t> get_intersect(const std::shared_ptr<c_cellview> &cv_ptr, cbag::level_t level,
                                     const cbag::box_t &test_box, cbag::offset_t spx,
                                     cbag::offset_t spy, bool no_sp) {
    auto ans = pyg::List<cbag::box_t>();
    auto &index = *(cv_ptr->get_geo_index(level));
    apply_intersect(index, [&ans](const auto &v) { ans.emplace_back(get_bbox(v)); }, test_box, spx,
                    spy, no_sp);
    return ans;
}

} // namespace lay
} // namespace pybag

namespace pl = pybag::lay;

void bind_inst_ref(py::module &m) {

    auto py_cls = py::class_<c_inst_ref>(m, "PyLayInstRef");
    py_cls.doc() = "A reference to a layout instance inside a cellview.";
    py_cls.def_property("nx", &pl::get_nx, &pl::set_nx, "Number of columns.");
    py_cls.def_property("ny", &pl::get_ny, &pl::set_ny, "Number of rows.");
    py_cls.def_property("spx", &pl::get_spx, &pl::set_spx, "Column pitch.");
    py_cls.def_property("spy", &pl::get_spy, &pl::set_spy, "Row pitch.");
    py_cls.def_property_readonly("xform", &pl::get_xform, "The Transform object.");
    py_cls.def_property_readonly(
        "inst_name", [](const c_inst_ref &ref) { return ref->get_inst_name(); }, "Instance name.");
    py_cls.def_property_readonly("committed", [](const c_inst_ref &ref) { return !ref.editable(); },
                                 "True if this instance is committed.");
    py_cls.def("move_by", &pl::move_by, "Moves the instance.", py::arg("dx"), py::arg("dy"));
    py_cls.def("transform", &pl::transform, "Transforms the instance.", py::arg("xform"));
    py_cls.def("set_master", &pl::set_master, "Sets the instance master.", py::arg("new_master"));
    py_cls.def("commit", &c_inst_ref::commit, "Commits the instance object.");
}

void bind_cellview(py::class_<c_cellview, std::shared_ptr<c_cellview>> &py_cls, py::module &m) {
    using c_tid = cbag::layout::track_id;
    using tup_int = pyg::Tuple<py::int_, py::int_>;

    py_cls.doc() = "A layout cellview.";

    py_cls.def(py::init<std::shared_ptr<const cbag::layout::routing_grid>,
                        std::shared_ptr<const cbag::layout::track_coloring>, std::string>(),
               "Construct a new cellview.", py::keep_alive<1, 2>(), py::keep_alive<1, 3>(),
               py::arg("grid"), py::arg("tr_colors"), py::arg("cell_name"));
    py_cls.def_property_readonly("is_empty", &c_cellview::empty, "True if this cellview is empty.");
    py_cls.def_property_readonly("cell_name", &c_cellview::get_name, "The cell name.");

    py_cls.def("__eq__", &c_cellview::operator==,
               "Returns True if the two layout cellviews are equal.", py::arg("other"));
    py_cls.def("set_grid", &c_cellview::set_grid, "Sets the routing grid.", py::arg("grid"));
    py_cls.def("get_rect_bbox", &cbag::layout::get_bbox,
               "Get the overall bounding box on the given layer.", py::arg("layer"),
               py::arg("purpose"));
    py_cls.def("add_prim_instance", &cbag::layout::add_prim_instance, "Adds a primitive instance.",
               py::arg("lib"), py::arg("cell"), py::arg("view"), py::arg("name"), py::arg("xform"),
               py::arg("nx"), py::arg("ny"), py::arg("spx"), py::arg("spy"), py::arg("commit"));
    py_cls.def("add_instance", &cbag::layout::add_instance, "Adds an instance",
               py::keep_alive<1, 2>(), py::arg("cv"), py::arg("name"), py::arg("xform"),
               py::arg("nx"), py::arg("ny"), py::arg("spx"), py::arg("spy"), py::arg("commit"));
    py_cls.def("add_rect", &cbag::layout::add_rect, "Adds a rectangle.", py::arg("layer"),
               py::arg("purpose"), py::arg("bbox"), py::arg("commit"));
    py_cls.def("add_rect_arr",
               [](c_cellview &cv, const std::string &layer, const std::string &purpose,
                  const cbag::box_t &box, cbag::cnt_t nx, cbag::cnt_t ny, cbag::offset_t spx,
                  cbag::offset_t spy) {
                   auto key = cbag::layout::layer_t_at(*(cv.get_tech()), layer, purpose);
                   cv.add_shape(key, c_box_arr{box, std::array<cbag::cnt_t, 2>{nx, ny},
                                               std::array<cbag::offset_t, 2>{spx, spy}});
               },
               "Adds an array of rectangles.", py::arg("layer"), py::arg("purpose"), py::arg("box"),
               py::arg("nx"), py::arg("ny"), py::arg("spx"), py::arg("spy"));
    py_cls.def("add_rect_arr",
               [](c_cellview &cv, const std::string &layer, const std::string &purpose,
                  const c_box_arr &barr) {
                   auto key = cbag::layout::layer_t_at(*(cv.get_tech()), layer, purpose);
                   cv.add_shape(key, barr);
               },
               "Adds an array of rectangles.", py::arg("layer"), py::arg("purpose"),
               py::arg("barr"));
    py_cls.def("add_rect_list",
               [](c_cellview &cv, const std::string &layer, const std::string &purpose,
                  const cbag::box_collection &bcol) {
                   auto key = cbag::layout::layer_t_at(*(cv.get_tech()), layer, purpose);
                   cv.add_shape(key, bcol);
               },
               "Adds a list of rectangles.", py::arg("layer"), py::arg("purpose"), py::arg("bcol"));

    py_cls.def("add_warr", &c_cellview::add_warr, "Adds a WireArray.", py::arg("tid"),
               py::arg("lower"), py::arg("upper"), py::arg("is_dummy") = false);
    py_cls.def("add_poly", &cbag::layout::add_poly<py_pt_vector>, "Adds a new polygon.",
               py::arg("layer"), py::arg("purpose"), py::arg("points"), py::arg("commit"));
    py_cls.def("add_blockage",
               [](const std::shared_ptr<c_cellview> &cv_ptr, const std::string &layer,
                  cbag::enum_t blk_code, const py_pt_vector &data, bool commit) {
                   return cbag::layout::add_blockage(
                       cv_ptr, layer, static_cast<cbag::blockage_type>(blk_code), data, commit);
               },
               "Adds a blockage object.", py::arg("layer"), py::arg("blk_code"), py::arg("points"),
               py::arg("commit"));
    py_cls.def("add_boundary",
               [](const std::shared_ptr<c_cellview> &cv_ptr, cbag::enum_t bnd_code,
                  const py_pt_vector &data, bool commit) {
                   return cbag::layout::add_boundary(
                       cv_ptr, static_cast<cbag::boundary_type>(bnd_code), data, commit);
               },
               "Adds a boundary object.", py::arg("bnd_code"), py::arg("points"),
               py::arg("commit"));
    py_cls.def("add_pin", &cbag::layout::add_pin, "Adds a pin object.", py::arg("layer"),
               py::arg("net"), py::arg("label"), py::arg("bbox"));
    py_cls.def("add_pin_arr", &cbag::layout::add_pin_arr, "Adds an arry of pins.", py::arg("net"),
               py::arg("label"), py::arg("tid"), py::arg("lower"), py::arg("upper"));
    py_cls.def("add_label", &cbag::layout::add_label, "Adds a label object.", py::arg("layer"),
               py::arg("purpose"), py::arg("xform"), py::arg("label"), py::arg("height"));
    py_cls.def("add_path",
               [](const std::shared_ptr<c_cellview> &cv_ptr, const std::string &layer,
                  const std::string &purpose, const py_pt_vector &data, cbag::offset_t half_width,
                  cbag::enum_t style0, cbag::enum_t style1, cbag::enum_t stylem, bool commit) {
                   return add_path(cv_ptr, layer, purpose, data, half_width,
                                   static_cast<cbag::end_style>(style0),
                                   static_cast<cbag::end_style>(style1),
                                   static_cast<cbag::end_style>(stylem), commit);
               },
               "Adds a new path.", py::arg("layer"), py::arg("purpose"), py::arg("points"),
               py::arg("half_width"), py::arg("style0"), py::arg("style1"), py::arg("stylem"),
               py::arg("commit"));
    py_cls.def("add_path45_bus",
               [](const std::shared_ptr<c_cellview> &cv_ptr, const std::string &layer,
                  const std::string &purpose, const py_pt_vector &data, pyg::List<int> widths,
                  pyg::List<int> spaces, cbag::enum_t style0, cbag::enum_t style1,
                  cbag::enum_t stylem, bool commit) {
                   return add_path45_bus(cv_ptr, layer, purpose, data, widths, spaces,
                                         static_cast<cbag::end_style>(style0),
                                         static_cast<cbag::end_style>(style1),
                                         static_cast<cbag::end_style>(stylem), commit);
               },
               "Adds a new 45 degree path bus.", py::arg("layer"), py::arg("purpose"),
               py::arg("points"), py::arg("widths"), py::arg("spaces"), py::arg("style0"),
               py::arg("style1"), py::arg("stylem"), py::arg("commit"));
    py_cls.def("add_via", &cbag::layout::add_via, "Add a via.", py::arg("xform"), py::arg("via_id"),
               py::arg("params"), py::arg("add_layers"), py::arg("commit"));
    py_cls.def(
        "add_via_arr",
        [](c_cellview &cv, const cbag::transformation &xform, const std::string &via_id,
           const cbag::layout::via_param &params, bool add_layers, cbag::cnt_t nx, cbag::cnt_t ny,
           cbag::offset_t spx, cbag::offset_t spy) {
            cbag::layout::add_via_arr(cv, xform, via_id, params, add_layers, {nx, ny}, {spx, spy});
        },
        "Add an array of vias.", py::arg("xform"), py::arg("via_id"), py::arg("params"),
        py::arg("add_layers"), py::arg("nx"), py::arg("ny"), py::arg("spx"), py::arg("spy"));
    py_cls.def("add_via_on_intersections",
               [](c_cellview &cv, const c_tid &tid1, const c_tid &tid2, cbag::coord_t l1,
                  cbag::coord_t u1, cbag::coord_t l2, cbag::coord_t u2, bool extend, bool contain) {
                   auto coord1 = std::array<cbag::coord_t, 2>{l1, u1};
                   auto coord2 = std::array<cbag::coord_t, 2>{l2, u2};
                   auto tmp = cbag::layout::add_via_on_intersections(cv, tid1, tid2, coord1, coord2,
                                                                     extend, contain);
                   return pyg::Tuple<tup_int, tup_int>::make_tuple(
                       tup_int::make_tuple(tmp[0][0], tmp[0][1]),
                       tup_int::make_tuple(tmp[1][0], tmp[1][1]));
               },
               "Add vias on the wire intersections.", py::arg("tid1"), py::arg("tid2"),
               py::arg("l1"), py::arg("u1"), py::arg("l2"), py::arg("u2"), py::arg("extend"),
               py::arg("contain"));
    py_cls.def("connect_barr_to_tracks",
               [](c_cellview &cv, cbag::enum_t lev_code, const std::string &layer,
                  const std::string &purpose, const c_box_arr &barr,
                  const cbag::layout::track_id &tid, std::optional<cbag::coord_t> tr_lower,
                  std::optional<cbag::coord_t> tr_upper, int min_len_code,
                  std::optional<cbag::coord_t> w_lower, std::optional<cbag::coord_t> w_upper) {
                   auto vdir = static_cast<cbag::direction_1d>(lev_code);
                   auto mode = static_cast<cbag::min_len_mode>(min_len_code);
                   auto key = cbag::layout::layer_t_at(*(cv.get_tech()), layer, purpose);
                   auto tmp = cbag::layout::connect_box_track(cv, vdir, key, barr.base, barr.num,
                                                              barr.sp, tid, {w_lower, w_upper},
                                                              {tr_lower, tr_upper}, mode);
                   return pyg::Tuple<tup_int, tup_int>::make_tuple(
                       tup_int::make_tuple(tmp[0][0], tmp[0][1]),
                       tup_int::make_tuple(tmp[1][0], tmp[1][1]));
               },
               "Connect the given BBoxArray to tracks.", py::arg("lev_code"), py::arg("layer"),
               py::arg("purpose"), py::arg("barr"), py::arg("tid"), py::arg("tr_lower"),
               py::arg("tr_upper"), py::arg("min_len_code"), py::arg("w_lower"),
               py::arg("w_upper"));
    py_cls.def("connect_warr_to_tracks",
               [](c_cellview &cv, const c_tid &w_tid, const c_tid &tid, cbag::coord_t w_lower,
                  cbag::coord_t w_upper) {
                   auto tmp = cbag::layout::connect_warr_track(cv, w_tid, tid, w_lower, w_upper);
                   return pyg::Tuple<tup_int, tup_int>::make_tuple(
                       tup_int::make_tuple(tmp[0][0], tmp[0][1]),
                       tup_int::make_tuple(tmp[1][0], tmp[1][1]));
               },
               "Connect the given WireArray to tracks.", py::arg("w_tid"), py::arg("tid"),
               py::arg("w_lower"), py::arg("w_upper"));
    py_cls.def("do_max_space_fill",
               [](c_cellview &cv, cbag::level_t level, const cbag::box_t &bbox, bool fill_bndry,
                  pyg::Tuple<int, int, int, int, double> fill_info) {
                   cv.do_max_space_fill(
                       level, bbox, fill_bndry,
                       std::array<cbag::offset_t, 2>{fill_info.get<0>(), fill_info.get<1>()},
                       std::array<cbag::offset_t, 2>{fill_info.get<2>(), fill_info.get<3>()},
                       fill_info.get<4>());
               },
               "Perform max space fill on the given layer.", py::arg("level"), py::arg("bbox"),
               py::arg("fill_boundary"), py::arg("fill_info"));
    py_cls.def("get_intersect", pybag::lay::get_intersect,
               "Get a list of bound boxes of all geometry intersecting the given box.",
               py::arg("layer"), py::arg("bnd_box"), py::arg("spx"), py::arg("spy"),
               py::arg("no_sp"));
}

void bind_layout(py::module &m) {
    auto cv_cls = py::class_<c_cellview, std::shared_ptr<c_cellview>>(m, "PyLayCellView");

    bind_inst_ref(m);
    bind_cellview(cv_cls, m);

    m.attr("COORD_MIN") = std::numeric_limits<cbag::coord_t>::min();
    m.attr("COORD_MAX") = std::numeric_limits<cbag::coord_t>::max();
}
