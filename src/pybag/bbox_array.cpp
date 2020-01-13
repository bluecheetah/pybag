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

#include <cstdlib>
#include <iterator>

#include <fmt/core.h>

#include <pybind11/stl.h>

#include <cbag/common/box_t.h>
#include <cbag/common/transformation_util.h>
#include <cbag/util/math.h>

#include <pybind11_generics/iterator.h>
#include <pybind11_generics/tuple.h>

#include <pybag/bbox.h>
#include <pybag/bbox_array.h>
#include <pybag/bbox_collection.h>
#include <pybag/enum_conv.h>

namespace pybag {
namespace util {

std::string to_string(const c_box_arr &self) {
    return fmt::format("BBoxArray({}, {}, {}, {}, {})", cbag::polygon::to_string(self.base),
                       self.num[0], self.num[1], self.sp[0], self.sp[1]);
}

coord_t get_coord(const c_box_arr &self, cbag::orientation_2d_t orient_code, bool bnd_code) {
    auto cur_coord =
        cbag::polygon::get_coord(self.base, static_cast<cbag::orientation_2d>(orient_code),
                                 static_cast<cbag::direction_1d>(bnd_code));
    offset_t delta = (self.num[orient_code] - 1) * self.sp[orient_code];
    if (delta < 0)
        bnd_code = 1 - bnd_code;
    return cur_coord + bnd_code * delta;
}

c_box get_bbox(const c_box_arr &self, cbag::cnt_t idx) {
    auto result = std::div(static_cast<long>(idx), static_cast<long>(self.num[0]));
    return cbag::polygon::get_move_by(self.base, result.rem * self.sp[0], result.quot * self.sp[1]);
}

c_box_arr &move_by(c_box_arr &self, offset_t dx, offset_t dy) {
    cbag::polygon::move_by(self.base, dx, dy);
    return self;
}

c_box_arr &transform(c_box_arr &self, const cbag::transformation &xform) {
    auto axis_xform = cbag::transformation(0, 0, xform.orient());
    auto pt = axis_xform.transform(self.sp[0], self.sp[1]);
    self.sp[0] = pt[0];
    self.sp[1] = pt[1];
    if (cbag::swaps_xy(xform.orient())) {
        std::swap(self.num[0], self.num[1]);
    }
    cbag::polygon::transform(self.base, xform);
    return self;
}

c_box_arr &extend_orient(c_box_arr &self, cbag::orientation_2d_t orient_code,
                         const std::optional<coord_t> &ct, const std::optional<coord_t> &cp) {
    bbox::extend_orient(self.base, orient_code, ct, cp);
    return self;
}

class box_arr_iter {
  public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = Py_ssize_t;
    using value_type = c_box;
    using reference = const value_type &;
    using pointer = const value_type *;

  private:
    const c_box_arr *parent_ = nullptr;
    cbag::cnt_t idx_ = 0;

  public:
    box_arr_iter() = default;
    box_arr_iter(const c_box_arr *parent, cbag::cnt_t idx) : parent_(parent), idx_(idx) {}

    box_arr_iter &operator++() {
        ++idx_;
        return *this;
    }

    value_type operator*() const { return get_bbox(*parent_, idx_); }

    bool operator==(const box_arr_iter &other) {
        return parent_ == other.parent_ && idx_ == other.idx_;
    }
    bool operator!=(const box_arr_iter &other) { return !(*this == other); }
};

pyg::PyIterator<c_box> get_box_iter(c_box_arr *barr_ptr) {
    return pyg::make_iterator(box_arr_iter(barr_ptr, 0),
                              box_arr_iter(barr_ptr, barr_ptr->num[0] * barr_ptr->num[1]));
}

} // namespace util
} // namespace pybag

namespace pu = pybag::util;

void bind_bbox_array(py::class_<c_box_arr> &py_cls) {

    pyg::declare_iterator<pu::box_arr_iter>();

    py_cls.doc() = "The bounding box array class.";
    py_cls.def(py::init([](const c_box &box, cbag::scnt_t nx, cbag::scnt_t ny, offset_t spx,
                           offset_t spy) {
                   if (nx <= 0 || ny <= 0)
                       throw std::invalid_argument(
                           fmt::format("nx = {} and ny = {} cannot be non-positive.", nx, ny));
                   return c_box_arr{box,
                                    std::array<cbag::cnt_t, 2>{static_cast<cbag::cnt_t>(nx),
                                                               static_cast<cbag::cnt_t>(ny)},
                                    std::array<offset_t, 2>{spx, spy}};
               }),
               "Construct a new BBoxArray.", py::arg("base"), py::arg("nx") = 1, py::arg("ny") = 1,
               py::arg("spx") = 0, py::arg("spy") = 0);
    py_cls.def(py::init([](const c_box &box, cbag::orientation_2d_t orient, cbag::scnt_t nt,
                           offset_t spt, cbag::scnt_t np, offset_t spp) {
                   if (nt <= 0 || np <= 0)
                       throw std::invalid_argument(
                           fmt::format("nt = {} and np = {} cannot be non-positive.", nt, np));

                   auto ans = c_box_arr{box, std::array<cbag::cnt_t, 2>{1, 1},
                                        std::array<offset_t, 2>{0, 0}};
                   ans.num[orient] = static_cast<cbag::cnt_t>(nt);
                   ans.num[orient ^ 1] = static_cast<cbag::cnt_t>(np);
                   ans.sp[orient] = spt;
                   ans.sp[orient ^ 1] = spp;
                   return ans;
               }),
               "Construct a new BBoxArray.", py::arg("base"), py::arg("orient_code"),
               py::arg("nt") = 1, py::arg("spt") = 0, py::arg("np") = 1, py::arg("spp") = 0);

    py_cls.def("__repr__", pu::to_string, "Returns a string representation of BBoxArray.");
    py_cls.def("__eq__",
               [](const c_box_arr &self, const c_box_arr &rhs) {
                   return self.base == rhs.base && self.num == rhs.num && self.sp == rhs.sp;
               },
               "Returns True if the two BBoxArray are equal.", py::arg("other"));
    py_cls.def("__iter__", &pu::get_box_iter, "Returns an iterator over BBox in this BBoxArray.");

    py_cls.def_readonly("base", &c_box_arr::base, "The base bounding box.");
    py_cls.def_property_readonly("nx", [](const c_box_arr &self) { return self.num[0]; },
                                 "Number of columns.");
    py_cls.def_property_readonly("ny", [](const c_box_arr &self) { return self.num[1]; },
                                 "Number of rows.");
    py_cls.def_property_readonly("spx", [](const c_box_arr &self) { return self.sp[0]; },
                                 "Column pitch.");
    py_cls.def_property_readonly("spy", [](const c_box_arr &self) { return self.sp[1]; },
                                 "Row pitch.");
    py_cls.def("get_num",
               [](const c_box_arr &self, cbag::orientation_2d_t orient_code) {
                   return self.num[orient_code];
               },
               "Get number of bbox in the given dimension.", py::arg("orient_code"));
    py_cls.def("get_sp",
               [](const c_box_arr &self, cbag::orientation_2d_t orient_code) {
                   return self.sp[orient_code];
               },
               "Get bbox pitch in the given dimension.", py::arg("orient_code"));
    py_cls.def("get_coord", &pu::get_coord, "Returns coordinate given orient/bound code.",
               py::arg("orient_code"), py::arg("bnd_code"));
    py_cls.def_property_readonly(
        "xl", [](const c_box_arr &self) { return pu::get_coord(self, 0, 0); }, "Left-most edge");
    py_cls.def_property_readonly(
        "xh", [](const c_box_arr &self) { return pu::get_coord(self, 0, 1); }, "Right-most edge");
    py_cls.def_property_readonly(
        "yl", [](const c_box_arr &self) { return pu::get_coord(self, 1, 0); }, "Bottom-most edge");
    py_cls.def_property_readonly(
        "yh", [](const c_box_arr &self) { return pu::get_coord(self, 1, 1); }, "Top-most edge");
    py_cls.def_property_readonly("xm",
                                 [](const c_box_arr &self) {
                                     return cbag::util::floor2(pu::get_coord(self, 0, 0) +
                                                               pu::get_coord(self, 0, 1));
                                 },
                                 "Center X coordinate");
    py_cls.def_property_readonly("ym",
                                 [](const c_box_arr &self) {
                                     return cbag::util::floor2(pu::get_coord(self, 1, 0) +
                                                               pu::get_coord(self, 1, 1));
                                 },
                                 "Center Y coordinate");
    py_cls.def_property_readonly("bound_box",
                                 [](const c_box_arr &self) {
                                     return c_box{
                                         pu::get_coord(self, 0, 0), pu::get_coord(self, 1, 0),
                                         pu::get_coord(self, 0, 1), pu::get_coord(self, 1, 1)};
                                 },
                                 "Overall BBox.");

    py_cls.def("get_array_info",
               [](const c_box_arr &self, cbag::orientation_2d_t orient_code) {
                   return pyg::Tuple<cbag::cnt_t, offset_t>::make_tuple(self.num[orient_code],
                                                                        self.sp[orient_code]);
               },
               "Returns num/pitch given orient code.", py::arg("orient_code"));

    py_cls.def("get_bbox", &pu::get_bbox, "Returns the BBox with the given index.", py::arg("idx"));

    py_cls.def("move_by", &pu::move_by, "Moves this BBoxArray.", py::arg("dx") = 0,
               py::arg("dy") = 0);
    py_cls.def("get_move_by",
               [](c_box_arr self, offset_t dx, offset_t dy) { return pu::move_by(self, dx, dy); },
               "Returns a moved BBoxArray.", py::arg("dx") = 0, py::arg("dy") = 0);
    py_cls.def("transform", &pu::transform, "Transforms this BBoxArray.", py::arg("xform"));
    py_cls.def("get_transform",
               [](c_box_arr self, const cbag::transformation &xform) {
                   return pu::transform(self, xform);
               },
               "Returns a transformed BBoxArray.", py::arg("xform"));
    py_cls.def("set_interval",
               [](c_box_arr &self, cbag::orientation_2d_t orient_code, coord_t lo, coord_t hi) {
                   set_interval(self.base, static_cast<cbag::orientation_2d>(orient_code), lo, hi);
                   return self;
               },
               "Set the interval of the basee BBox of this BBoxArray.", py::arg("orient_code"),
               py::arg("lo"), py::arg("hi"));
    py_cls.def("extend_orient", &pu::extend_orient, "Extends this BBoxArray.",
               py::arg("orient_code"), py::arg("ct") = py::none(), py::arg("cp") = py::none());
    py_cls.def("get_extend_orient",
               [](c_box_arr self, cbag::orientation_2d_t orient_code,
                  const std::optional<coord_t> &ct, const std::optional<coord_t> &cp) {
                   return pu::extend_orient(self, orient_code, ct, cp);
               },
               "Returns an extended BBoxArray.", py::arg("orient_code"), py::arg("ct") = py::none(),
               py::arg("cp") = py::none());
    py_cls.def("get_copy", [](c_box_arr self) { return self; },
               "Returns a copy of this BBoxArray.");
    py_cls.def("get_sub_array",
               [](const c_box_arr &self, cbag::orientation_2d_t orient_code, cbag::scnt_t div,
                  cbag::scnt_t idx) {
                   auto n_tot = self.num[orient_code];
                   auto pitch = self.sp[orient_code];
                   auto n_par = self.num[orient_code ^ 1];
                   auto sp_par = self.sp[orient_code ^ 1];

                   auto q = static_cast<cbag::scnt_t>(n_tot / div);
                   auto r = static_cast<cbag::scnt_t>(n_tot % div);

                   auto new_n = q + (idx < r);

                   auto ans = c_box_arr{
                       pybag::bbox::get_move_by_orient(self.base, orient_code, idx * pitch, 0),
                       std::array<cbag::cnt_t, 2>{1, 1}, std::array<offset_t, 2>{0, 0}};
                   ans.num[orient_code] = new_n;
                   ans.num[orient_code ^ 1] = n_par;
                   ans.sp[orient_code] = static_cast<offset_t>(pitch * div);
                   ans.sp[orient_code ^ 1] = sp_par;
                   return ans;
               },
               "Returns a new BBoxArray that is a sub-array of this one.", py::arg("orient_code"),
               py::arg("div_num"), py::arg("idx"));
}
