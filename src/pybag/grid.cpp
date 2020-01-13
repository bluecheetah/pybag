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

#include <pybind11_generics/iterator.h>
#include <pybind11_generics/list.h>
#include <pybind11_generics/optional.h>
#include <pybind11_generics/tuple.h>

#include <cbag/common/transformation_util.h>
#include <cbag/layout/grid_object.h>
#include <cbag/layout/routing_grid.h>
#include <cbag/layout/routing_grid_util.h>
#include <cbag/layout/tech_util.h>
#include <cbag/layout/track_coloring.h>
#include <cbag/layout/track_info_util.h>
#include <cbag/layout/wire_width.h>

#include <pybag/enum_conv.h>
#include <pybag/grid.h>

using c_tech = cbag::layout::tech;
using c_grid = cbag::layout::routing_grid;

namespace pybag {
namespace tech {

class py_warr_rect_iterator {
  private:
    cbag::layout::warr_rect_iterator iter;

  public:
    py_warr_rect_iterator() = default;

    py_warr_rect_iterator(cbag::layout::warr_rect_iterator &&iter) : iter(std::move(iter)) {}

    bool operator==(const py_warr_rect_iterator &rhs) const { return iter == rhs.iter; }
    bool operator!=(const py_warr_rect_iterator &rhs) const { return iter != rhs.iter; }

    pyg::Tuple<py::str, py::str, cbag::box_t> operator*() const {
        auto[lay_t, box] = *iter;
        auto &tech = *(iter.get_tech());
        return pyg::Tuple<py::str, py::str, cbag::box_t>::make_tuple(
            tech.get_layer_name(lay_t.first), tech.get_purpose_name(lay_t.second), box);
    }
    py_warr_rect_iterator &operator++() {
        ++iter;
        return *this;
    }
};

std::shared_ptr<c_grid> make_grid(const std::shared_ptr<const c_tech> &tech_ptr,
                                  const std::string &fname) {
    return std::make_shared<c_grid>(cbag::layout::make_grid(tech_ptr, fname));
}

} // namespace tech
} // namespace pybag

void bind_track_info(py::module &m) {
    using c_tr_info = cbag::layout::track_info;

    auto py_cls = py::class_<c_tr_info>(m, "TrackInfo");
    py_cls.doc() = "A class containing routing grid information on a specific layer.";
    py_cls.def("__eq__", &c_tr_info::operator==, py::arg("other"));
    py_cls.def("__hash__", &c_tr_info::get_hash);
    py_cls.def_property_readonly("pitch", &c_tr_info::get_pitch, "Track pitch.");
    py_cls.def_property_readonly("width", &c_tr_info::get_width, "Track width.");
    py_cls.def_property_readonly("space", &c_tr_info::get_space, "Track space.");
    py_cls.def_property_readonly("offset", &c_tr_info::get_offset, "Track offset.");
    py_cls.def("compatible", &c_tr_info::compatible,
               "True if the two TrackInfo objects are compatible.", py::arg("other"));
}

void bind_track_coloring(py::module &m) {
    using c_tr_color = cbag::layout::track_coloring;

    auto py_cls = py::class_<c_tr_color, std::shared_ptr<c_tr_color>>(m, "TrackColoring");
    py_cls.doc() = "A class containing routing grid flip parity information.";
    py_cls.def("__eq__", &c_tr_color::operator==, py::arg("other"));
    py_cls.def("__hash__", &c_tr_color::get_hash);
    py_cls.def("__repr__", &c_tr_color::to_string);
    py_cls.def("get_htr_parity", &c_tr_color::get_htr_parity,
               "Get parity of the given half-track index.", py::arg("level"), py::arg("htr"));
}

void bind_track_id(py::module &m) {
    using c_tid = cbag::layout::track_id;

    auto py_cls = py::class_<c_tid>(m, "PyTrackID");
    py_cls.doc() = "A class containing track index information.";
    py_cls.def(py::init<cbag::level_t, cbag::htr_t, cbag::cnt_t, cbag::cnt_t, cbag::htr_t>(),
               "Create a new PyTrackID object.", py::arg("layer_id"), py::arg("htr"),
               py::arg("ntr"), py::arg("num"), py::arg("htr_pitch"));
    py_cls.def("__eq__", &c_tid::operator==, py::arg("other"));
    py_cls.def("__hash__", &c_tid::get_hash);
    py_cls.def("__repr__", &c_tid::to_string);

    py_cls.def_property_readonly("layer_id", &c_tid::get_level, "The layer ID.");
    py_cls.def_property_readonly("width", &c_tid::get_ntr, "The wire width in number of tracks.");
    py_cls.def_property_readonly("num", &c_tid::get_num, "Number of wires in this PyTrackID.");
    py_cls.def_property("base_htr", &c_tid::get_htr, &c_tid::set_htr, "The half-track index.");
    py_cls.def_property("htr_pitch", &c_tid::get_pitch, &c_tid::set_pitch,
                        "Pitch betweek wires in half-tracks.");
    py_cls.def("get_bounds",
               [](const c_tid &self, const c_grid &grid) {
                   auto ans = self.get_bounds(grid);
                   return pyg::Tuple<py::int_, py::int_>::make_tuple(ans[0], ans[1]);
               },
               "Returns the bounds of this track ID.", py::arg("grid"));
}

void bind_routing_grid(py::module &m) {
    using tr_specs_t = pyg::List<pyg::Tuple<int, int, int, int, int>>;
    using c_tr_color = cbag::layout::track_coloring;
    using c_tid = cbag::layout::track_id;

    bind_track_info(m);
    bind_track_coloring(m);

    auto py_cls = py::class_<c_grid, std::shared_ptr<c_grid>>(m, "PyRoutingGrid");

    bind_track_id(m);

    py_cls.doc() = "The routing grid class.";
    py_cls.def(py::init(&pybag::tech::make_grid), "Create a new PyRoutingGrid class from file.",
               py::keep_alive<1, 2>(), py::arg("tech"), py::arg("fname"));
    py_cls.def(py::init<const c_grid &>(),
               "Create a new PyRoutingGrid class from another RoutingGrid.", py::arg("grid"));
    py_cls.def("__eq__", &c_grid::operator==, py::arg("other"));
    py_cls.def("__hash__", &c_grid::get_hash);
    py_cls.def_property_readonly("top_ignore_layer", &c_grid::get_top_ignore_level,
                                 "The top ignore layer ID.");
    py_cls.def_property_readonly("bot_layer", &c_grid::get_bot_level, "The bottom layer ID.");
    py_cls.def_property_readonly("top_layer", &c_grid::get_top_level, "The top layer ID.");
    py_cls.def_property_readonly("top_private_layer", &c_grid::get_top_private_level,
                                 "The top private layer ID.");
    py_cls.def_property_readonly("resolution",
                                 [](const c_grid &g) { return g.get_tech()->get_resolution(); },
                                 "The layout resolution.");
    py_cls.def_property_readonly("layout_unit",
                                 [](const c_grid &g) { return g.get_tech()->get_layout_unit(); },
                                 "The layout resolution.");
    py_cls.def("get_track_info", &c_grid::track_info_at,
               "Returns the TrackInfo object on the given layer.", py::arg("lay_id"));
    py_cls.def("get_direction",
               [](const c_grid &g, cbag::level_t lay_id) -> pyg::PyOrient2D {
                   return pybag::util::code_to_orient_2d(static_cast<cbag::orientation_2d_t>(
                       g.track_info_at(lay_id).get_direction()));
               },
               "Returns the track direction for the given layer.", py::arg("lay_id"));
    py_cls.def(
        "get_track_offset",
        [](const c_grid &g, cbag::level_t lay_id) { return g.track_info_at(lay_id).get_offset(); },
        "Returns the track offset for the given layer.", py::arg("lay_id"));
    py_cls.def(
        "get_track_pitch",
        [](const c_grid &g, cbag::level_t lay_id) { return g.track_info_at(lay_id).get_pitch(); },
        "Returns the track pitch for the given layer.", py::arg("lay_id"));
    py_cls.def("get_min_cont_length",
               [](const c_grid &g, cbag::level_t lay_id, cbag::cnt_t num_tr) {
                   auto &tech = *g.get_tech();
                   auto &tinfo = g.track_info_at(lay_id);
                   auto tr_dir = tinfo.get_direction();
                   auto cur_len = tech.get_width_intervals(lay_id, tr_dir).at_back()[0];
                   return tech.get_next_length(lay_id, tr_dir, g.get_wire_width(lay_id, num_tr),
                                               cur_len, false);
               },
               "Returns the minimum length for which all longer lengths are legal.",
               py::arg("lay_id"), py::arg("num_tr"));
    py_cls.def("get_next_length",
               [](const c_grid &g, cbag::level_t lay_id, cbag::cnt_t num_tr, cbag::offset_t cur_len,
                  bool even) {
                   auto &tinfo = g.track_info_at(lay_id);
                   auto wire_w = g.get_wire_width(lay_id, num_tr);
                   return g.get_tech()->get_next_length(lay_id, tinfo.get_direction(), wire_w,
                                                        cur_len, even);
               },
               "Returns the next legal length.", py::arg("lay_id"), py::arg("num_tr"),
               py::arg("cur_len"), py::arg("even") = false);
    py_cls.def("get_prev_length",
               [](const c_grid &g, cbag::level_t lay_id, cbag::cnt_t num_tr, cbag::offset_t cur_len,
                  bool even) {
                   auto &tinfo = g.track_info_at(lay_id);
                   auto wire_w = g.get_wire_width(lay_id, num_tr);
                   return g.get_tech()->get_prev_length(lay_id, tinfo.get_direction(), wire_w,
                                                        cur_len, even);
               },
               "Returns the previous legal length.", py::arg("lay_id"), py::arg("num_tr"),
               py::arg("cur_len"), py::arg("even") = false);
    py_cls.def(
        "get_space",
        [](const c_grid &g, cbag::level_t lay_id, cbag::cnt_t num_tr, bool same_color, bool even) {
            auto sp_type = cbag::get_space_type(same_color);
            auto wire_w = g.get_wire_width(lay_id, num_tr);
            return cbag::layout::get_min_space(*g.get_tech(), lay_id, wire_w, sp_type, even);
        },
        "Returns the track pitch for the given layer.", py::arg("lay_id"), py::arg("num_tr"),
        py::arg("same_color") = false, py::arg("even") = false);
    py_cls.def("get_line_end_space",
               [](const c_grid &g, cbag::level_t lay_id, cbag::cnt_t num_tr, bool even) {
                   auto wire_w = g.get_wire_width(lay_id, num_tr);
                   return cbag::layout::get_min_space(*g.get_tech(), lay_id, wire_w,
                                                      cbag::space_type::LINE_END, even);
               },
               "Returns the track pitch for the given layer.", py::arg("lay_id"), py::arg("num_tr"),
               py::arg("even") = false);
    py_cls.def(
        "get_min_space",
        [](const c_grid &g, cbag::level_t lay_id, cbag::cnt_t num_tr, bool same_color, bool even) {
            return cbag::layout::get_min_space(g, lay_id, num_tr, same_color, even);
        },
        "Returns the minimum space required around the given wire in resolution units.",
        py::arg("lay_id"), py::arg("num_tr"), py::arg("same_color") = false,
        py::arg("even") = false);
    py_cls.def("get_via_extensions",
               [](const c_grid &g, cbag::enum_t lev_code, cbag::level_t lay_id, cbag::cnt_t ntr,
                  cbag::cnt_t adj_ntr) {
                   auto tmp = cbag::layout::get_via_extensions(
                       g, static_cast<cbag::direction_1d>(lev_code), lay_id, ntr, adj_ntr);
                   return pyg::Tuple<py::int_, py::int_>::make_tuple(tmp[0], tmp[1]);
               },
               "Returns the via extensions. Index 0 is extension on lower level.",
               py::arg("lev_code"), py::arg("lay_id"), py::arg("num_tr"), py::arg("adj_num_tr"));
    py_cls.def("get_via_extensions_dim",
               [](const c_grid &g, cbag::enum_t lev_code, cbag::level_t lay_id, cbag::offset_t dim,
                  cbag::offset_t adj_dim) {
                   auto tmp = cbag::layout::get_via_extensions_dim(
                       g, static_cast<cbag::direction_1d>(lev_code), lay_id, dim, adj_dim);
                   return pyg::Tuple<py::int_, py::int_>::make_tuple(tmp[0], tmp[1]);
               },
               "Returns the via extensions. Index 0 is extension on lower level.",
               py::arg("lev_code"), py::arg("lay_id"), py::arg("dim"), py::arg("adj_dim"));
    py_cls.def("get_via_extensions_dim_tr",
               [](const c_grid &g, cbag::enum_t lev_code, cbag::level_t lay_id, cbag::offset_t dim,
                  cbag::cnt_t adj_ntr) {
                   auto tmp = cbag::layout::get_via_extensions_dim_tr(
                       g, static_cast<cbag::direction_1d>(lev_code), lay_id, dim, adj_ntr);
                   return pyg::Tuple<py::int_, py::int_>::make_tuple(tmp[0], tmp[1]);
               },
               "Returns the via extensions. Index 0 is extension on lower level.",
               py::arg("lev_code"), py::arg("lay_id"), py::arg("dim"), py::arg("adj_num_tr"));
    py_cls.def("get_sep_htr", &cbag::layout::get_sep_htr,
               "Returns the half-track separation needed between two wires.", py::arg("lay_id"),
               py::arg("ntr1"), py::arg("ntr2"), py::arg("same_color"));
    py_cls.def(
        "get_line_end_sep_htr",
        [](const c_grid &g, cbag::enum_t lev_code, cbag::level_t lay_id, cbag::cnt_t ntr,
           cbag::cnt_t adj_ntr) {
            return cbag::layout::get_line_end_sep_htr(g, static_cast<cbag::direction_1d>(lev_code),
                                                      lay_id, ntr, adj_ntr);
        },
        "Returns the half-track separation to satisfy via extension + line-end spacing constraint.",
        py::arg("lev_code"), py::arg("lay_id"), py::arg("ntr"), py::arg("adj_ntr"));
    py_cls.def("get_block_size",
               [](const c_grid &g, cbag::level_t level, bool include_private, bool half_blk_x,
                  bool half_blk_y) {
                   std::array<bool, 2> half_blk = {half_blk_x, half_blk_y};
                   auto ans = cbag::layout::get_blk_size(g, level, include_private, half_blk);
                   return pyg::Tuple<py::int_, py::int_>::make_tuple(ans[0], ans[1]);
               },
               "Returns the unit block size given top routing level.", py::arg("layer_id"),
               py::arg("include_private") = false, py::arg("half_blk_x") = false,
               py::arg("half_blk_y") = false);
    py_cls.def("size_defined", &cbag::layout::block_defined_at,
               "Returns True if both width and height are quantized at the given layer.",
               py::arg("layer_id"));
    py_cls.def("get_size_pitch",
               [](const c_grid &g, cbag::level_t lay_id) {
                   auto ans = cbag::layout::get_top_track_pitches(g, lay_id);
                   return pyg::Tuple<py::int_, py::int_>::make_tuple(ans[0], ans[1]);
               },
               "Returns the pitches that defines template size.", py::arg("layer_id"));
    py_cls.def("get_track_coloring_at", &c_grid::get_track_coloring_at,
               "Gets the track_coloring information at the given location.", py::arg("tr_colors"),
               py::arg("xform"), py::arg("child"), py::arg("top_layer"));
    py_cls.def("get_wire_total_width",
               [](const c_grid &g, cbag::level_t lay_id, cbag::cnt_t ntr) {
                   auto &tinfo = g.track_info_at(lay_id);
                   return g.get_wire_width(lay_id, ntr).get_total_width(tinfo.get_pitch() / 2);
               },
               "Returns the total width of the given wire bundle.", py::arg("layer_id"),
               py::arg("ntr"));
    py_cls.def("get_wire_bounds_htr",
               [](const c_grid &g, cbag::level_t lay_id, cbag::htr_t htr, cbag::cnt_t ntr) {
                   auto ans = cbag::layout::get_wire_bounds(g, lay_id, htr, ntr);
                   return pyg::Tuple<py::int_, py::int_>::make_tuple(ans[0], ans[1]);
               },
               "Returns the wire boundary coordinates.", py::arg("layer_id"), py::arg("htr"),
               py::arg("ntr"));
    py_cls.def("get_wire_em_specs", &cbag::layout::get_wire_em_specs, "Returns the wire EM specs.",
               py::arg("layer_id"), py::arg("num_tr"), py::arg("length") = -1,
               py::arg("vertical") = false, py::arg("dc_temp") = -1000, py::arg("rms_dt") = -1000);
    py_cls.def("get_min_track_width", &cbag::layout::get_min_num_tr,
               "Returns the minimum number of tracks required for the given EM specs or connection "
               "to lower/upper layers.",
               py::arg("layer_id"), py::arg("idc") = 0, py::arg("iac_rms") = 0,
               py::arg("iac_peak") = 0, py::arg("length") = -1, py::arg("bot_ntr") = 0,
               py::arg("top_ntr") = 0, py::arg("dc_temp") = -1000, py::arg("rms_dt") = -1000);
    py_cls.def("coord_to_htr",
               [](const c_grid &g, cbag::level_t lay_id, cbag::offset_t coord,
                  cbag::senum_t round_mode, bool even) {
                   return cbag::layout::coord_to_htr(g.track_info_at(lay_id), coord,
                                                     static_cast<cbag::round_mode>(round_mode),
                                                     even);
               },
               "Convert coordinate to half-track index.", py::arg("layer_id"), py::arg("coord"),
               py::arg("round_mode"), py::arg("even"));
    py_cls.def(
        "find_next_htr",
        [](const c_grid &g, cbag::level_t lay_id, cbag::offset_t coord, cbag::cnt_t ntr,
           cbag::senum_t round_mode, bool even) {
            return cbag::layout::find_next_htr(g, lay_id, coord, ntr,
                                               static_cast<cbag::round_mode>(round_mode), even);
        },
        "Find the wire htr index that have both edges greater/less than the given coordinate.",
        py::arg("layer_id"), py::arg("coord"), py::arg("ntr"), py::arg("round_mode"),
        py::arg("even"));
    py_cls.def("htr_to_coord",
               [](const c_grid &g, cbag::level_t lay_id, cbag::htr_t htr) {
                   return cbag::layout::htr_to_coord(g.track_info_at(lay_id), htr);
               },
               "Convert half-track index to coordinate.", py::arg("layer_id"), py::arg("htr"));
    py_cls.def("transform_htr",
               [](const c_grid &g, cbag::level_t lay_id, cbag::htr_t htr,
                  const cbag::transformation &xform) {
                   return cbag::layout::transform_htr(g.track_info_at(lay_id), htr, xform);
               },
               "Transforms the given half-track index.", py::arg("layer_id"), py::arg("htr"),
               py::arg("xform"));
    py_cls.def("get_copy_with",
               [](const c_grid &g, cbag::level_t top_ignore_lay, cbag::level_t top_private_lay,
                  tr_specs_t tr_specs) {
                   return g.get_copy_with(top_ignore_lay, top_private_lay, tr_specs);
               },
               "Returns a modified copy of the routing grid.", py::arg("top_ignore_lay"),
               py::arg("top_private_lay"), py::arg("tr_specs"));

    m.def("coord_to_custom_htr",
          [](cbag::offset_t coord, cbag::offset_t pitch, cbag::offset_t off,
             cbag::enum_t round_mode, bool even) {
              return cbag::layout::coord_to_htr(coord, pitch, off,
                                                static_cast<cbag::round_mode>(round_mode), even);
          },
          "Convert coordinate to half-track index given pitch/offset.", py::arg("coord"),
          py::arg("pitch"), py::arg("off"), py::arg("round_mode"), py::arg("even"));

    m.def("make_tr_colors",
          [](const c_tech &t) {
              return std::make_shared<const c_tr_color>(cbag::layout::make_track_coloring(t));
          },
          "Create a new TrackColoring object.", py::arg("tech"));

    pyg::declare_iterator<pybag::tech::py_warr_rect_iterator>();

    m.def("get_wire_iterator",
          [](const c_grid &grid, const c_tr_color &colors, const c_tid &tid, cbag::coord_t lower,
             cbag::coord_t upper) -> pyg::PyIterator<pyg::Tuple<py::str, py::str, cbag::box_t>> {
              return pyg::make_iterator(
                  pybag::tech::py_warr_rect_iterator(
                      cbag::layout::begin_rect(grid, colors, tid, lower, upper)),
                  pybag::tech::py_warr_rect_iterator(cbag::layout::end_rect(tid)));
          },
          "Returns an iterator over layer/purpose/BBox.", py::arg("grid"), py::arg("tr_colors"),
          py::arg("tid"), py::arg("lower"), py::arg("upper"));
}
