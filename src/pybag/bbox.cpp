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
#include <utility>

#include <pybind11/stl.h>

#include <cbag/common/box_t.h>
#include <cbag/common/transformation_util.h>
#include <cbag/polygon/enum.h>

#include <pybind11_generics/custom.h>
#include <pybind11_generics/tuple.h>

#include <pybag/bbox.h>
#include <pybag/bbox_array.h>
#include <pybag/bbox_collection.h>
#include <pybag/enum_conv.h>

namespace pyg = pybind11_generics;

namespace pybag {
namespace bbox {

c_box &extend(c_box &self, std::optional<coord_t> x, std::optional<coord_t> y) {
    auto xc = (x) ? *x : xl(self);
    auto yc = (y) ? *y : yl(self);
    extend_to(self, xc, yc);
    return self;
}

c_box get_extend(c_box self, std::optional<coord_t> x, std::optional<coord_t> y) {
    return extend(self, x, y);
}

c_box &extend_orient(c_box &self, cbag::orientation_2d_t orient_code, std::optional<coord_t> ct,
                     std::optional<coord_t> cp) {
    auto arr = std::array<std::optional<coord_t>, 2>{};
    arr[orient_code] = ct;
    arr[orient_code ^ 1] = cp;
    return extend(self, arr[0], arr[1]);
}

c_box get_extend_orient(c_box self, cbag::orientation_2d_t orient_code, std::optional<coord_t> ct,
                        std::optional<coord_t> cp) {
    return extend_orient(self, orient_code, ct, cp);
}

c_box &expand(c_box &self, coord_t dx, coord_t dy) {
    set_interval(self, cbag::orientation_2d::X, xl(self) - dx, xh(self) + dx);
    set_interval(self, cbag::orientation_2d::Y, yl(self) - dy, yh(self) + dy);
    return self;
}

c_box get_expand(c_box self, coord_t dx, coord_t dy) { return expand(self, dx, dy); }

c_box &transform(c_box &self, const cbag::transformation &xform) {
    return cbag::polygon::transform(self, xform);
}

c_box get_transform(const c_box &self, const cbag::transformation &xform) {
    return cbag::polygon::get_transform(self, xform);
}

c_box &move_by(c_box &self, coord_t dx, coord_t dy) { return cbag::polygon::move_by(self, dx, dy); }

c_box get_move_by(const c_box &self, coord_t dx, coord_t dy) {
    return cbag::polygon::get_move_by(self, dx, dy);
}

c_box &move_by_orient(c_box &self, cbag::orientation_2d_t ocode, coord_t dt, coord_t dp) {
    return cbag::polygon::move_by_orient(self, static_cast<cbag::orientation_2d>(ocode), dt, dp);
}

c_box get_move_by_orient(c_box self, cbag::orientation_2d_t ocode, coord_t dx, coord_t dy) {
    return move_by_orient(self, ocode, dx, dy);
}

c_box &flip_xy(c_box &self) {
    auto x0 = xl(self);
    auto x1 = xh(self);
    auto y0 = yl(self);
    auto y1 = yh(self);
    set_interval(self, cbag::orientation_2d::X, y0, y1);
    set_interval(self, cbag::orientation_2d::Y, x0, x1);
    return self;
}

c_box get_flip_xy(c_box self) { return flip_xy(self); }

} // namespace bbox
} // namespace pybag

namespace pu = pybag::util;

void bind_bbox(py::class_<c_box> &py_cls) {
    py_cls.doc() = "The bounding box class.";
    py_cls.def(py::init<coord_t, coord_t, coord_t, coord_t>(), "Construct a new BBox.",
               py::arg("xl"), py::arg("yl"), py::arg("xh"), py::arg("yh"));
    py_cls.def(py::init([](cbag::orientation_2d_t orient_code, coord_t tl, coord_t th, coord_t pl,
                           coord_t ph) {
                   return c_box{static_cast<cbag::orientation_2d>(orient_code), tl, th, pl, ph};
               }),
               "Construct a new BBox from orientation.", py::arg("orient_code"), py::arg("tl"),
               py::arg("th"), py::arg("pl"), py::arg("ph"));

    py_cls.def("__repr__", [](const c_box &obj) { return to_string(obj); },
               "Returns a string representation of BBox.");
    py_cls.def("__eq__", &c_box::operator==, "Returns True if the two BBox are equal.",
               py::arg("other"));

    py_cls.def_static("get_invalid_bbox", &c_box::get_invalid_bbox, "Create an invalid BBox.");
    py_cls.def_property_readonly("xl", &cbag::polygon::xl<c_box>, "Left coordinate.");

    py_cls.def_property_readonly("yl", &cbag::polygon::yl<c_box>, "Bottom coordinate.");
    py_cls.def_property_readonly("xh", &cbag::polygon::xh<c_box>, "Right coordinate.");
    py_cls.def_property_readonly("yh", &cbag::polygon::yh<c_box>, "Top coordinate.");
    py_cls.def_property_readonly("xm", &cbag::polygon::xm<c_box>, "Center X coordinate.");
    py_cls.def_property_readonly("ym", &cbag::polygon::ym<c_box>, "Center Y coordinate.");
    py_cls.def_property_readonly("w", &cbag::polygon::width<c_box>, "Width.");
    py_cls.def_property_readonly("h", &cbag::polygon::height<c_box>, "Height.");

    /*
    py_cls.def_property_readonly("left_unit", &c_box::xl, "Left coordinate.");
    py_cls.def_property_readonly("bottom_unit", &c_box::yl, "Bottom coordinate.");
    py_cls.def_property_readonly("right_unit", &c_box::xh, "Right coordinate.");
    py_cls.def_property_readonly("top_unit", &c_box::yh, "Top coordinate.");
    py_cls.def_property_readonly("xc_unit", &c_box::xm, "Center X coordinate.");
    py_cls.def_property_readonly("yc_unit", &c_box::ym, "Center Y coordinate.");
    py_cls.def_property_readonly("width_unit", &c_box::w, "Width.");
    py_cls.def_property_readonly("height_unit", &c_box::h, "Height.");
    */

    py_cls.def("get_dim",
               [](const c_box &self, cbag::orientation_2d_t orient_code) {
                   return dimension(self, static_cast<cbag::orientation_2d>(orient_code));
               },
               "Returns the dimension along the given orientation.", py::arg("orient_code"));
    py_cls.def("get_coord",
               [](const c_box &self, cbag::orientation_2d_t orient_code, bool bnd_code) {
                   return get_coord(self, static_cast<cbag::orientation_2d>(orient_code),
                                    static_cast<cbag::direction_1d>(bnd_code));
               },
               "Returns coordinate given orient/bound code.", py::arg("orient_code"),
               py::arg("bnd_code"));
    py_cls.def("get_center",
               [](const c_box &self, cbag::orientation_2d_t orient_code) {
                   return center(self, static_cast<cbag::orientation_2d>(orient_code));
               },
               "Returns center coordinate given orient code.", py::arg("orient_code"));
    py_cls.def("get_interval",
               [](const c_box &self, cbag::orientation_2d_t orient_code) {
                   auto orient = static_cast<cbag::orientation_2d>(orient_code);
                   return pyg::Tuple<coord_t, coord_t>::make_tuple(lower(self, orient),
                                                                   upper(self, orient));
               },
               "Returns interval given orient code.", py::arg("orient_code"));

    py_cls.def("is_physical", &cbag::polygon::is_physical<c_box>,
               "True if this BBox has positive area.");
    py_cls.def("is_valid", &cbag::polygon::is_valid<c_box>,
               "True if this BBox has nonnegative area.");
    py_cls.def("overlaps",
               [](const c_box &self, c_box bbox) {
                   bbox &= self;
                   return is_physical(bbox);
               },
               "True if the two BBox overlaps.", py::arg("bbox"));

    py_cls.def("merge", [](c_box &self, const c_box &bbox) -> c_box & { return self |= bbox; },
               "Set to union of BBox", py::arg("bbox"));
    py_cls.def("get_merge", [](const c_box &self, c_box bbox) { return bbox |= self; },
               "Get union of BBox", py::arg("bbox"));
    py_cls.def("intersect", [](c_box &self, const c_box &bbox) -> c_box & { return self &= bbox; },
               "Set to intersection of BBox", py::arg("bbox"));
    py_cls.def("get_intersect", [](const c_box &self, c_box bbox) { return bbox &= self; },
               "Get intersection of BBox", py::arg("bbox"));
    py_cls.def("extend", &pybag::bbox::extend, "Extend BBox to the given coordinates.",
               py::arg("x") = py::none(), py::arg("y") = py::none());
    py_cls.def("get_extend", &pybag::bbox::get_extend,
               "Returns an extended BBox to the given coordinates.", py::arg("x") = py::none(),
               py::arg("y") = py::none());
    py_cls.def(
        "set_interval",
        [](c_box &self, cbag::orientation_2d_t orient_code, coord_t lo, coord_t hi) -> c_box & {
            set_interval(self, static_cast<cbag::orientation_2d>(orient_code), lo, hi);
            return self;
        },
        "Set the interval of this BBox.", py::arg("orient_code"), py::arg("lo"), py::arg("hi"));
    py_cls.def("extend_orient", &pybag::bbox::extend_orient, "Extends this BBox.",
               py::arg("orient_code"), py::arg("ct") = py::none(), py::arg("cp") = py::none());
    py_cls.def("get_extend_orient", &pybag::bbox::get_extend_orient, "Returns an extended BBox.",
               py::arg("orient_code"), py::arg("ct") = py::none(), py::arg("cp") = py::none());
    py_cls.def("expand", &pybag::bbox::expand, "Expand BBox (on all sides).", py::arg("dx") = 0,
               py::arg("dy") = 0);
    py_cls.def("get_expand", &pybag::bbox::get_expand, "Returns an expanded BBox (on all sides).",
               py::arg("dx") = 0, py::arg("dy") = 0);
    py_cls.def("transform", &pybag::bbox::transform, "Transforms the BBox.", py::arg("xform"));
    py_cls.def("get_transform", &pybag::bbox::get_transform, "Returns a transformed BBox.",
               py::arg("xform"));
    py_cls.def("move_by", &pybag::bbox::move_by, "Moves the BBox.", py::arg("dx") = 0,
               py::arg("dy") = 0);
    py_cls.def("get_move_by", &pybag::bbox::get_move_by, "Returns a moved BBox.", py::arg("dx") = 0,
               py::arg("dy") = 0);
    py_cls.def("move_by_orient", &pybag::bbox::move_by_orient, "Moves the BBox.",
               py::arg("orient_code"), py::arg("dt") = 0, py::arg("dp") = 0);
    py_cls.def("get_move_by_orient", &pybag::bbox::get_move_by_orient, "Returns a moved BBox.",
               py::arg("orient_code"), py::arg("dt") = 0, py::arg("dp") = 0);
    py_cls.def("flip_xy", &pybag::bbox::flip_xy, "Flips the BBox X and Y coordinates.");
    py_cls.def("get_flip_xy", &pybag::bbox::get_flip_xy,
               "Returns a flipped the BBox X and Y coordinates.");

    py_cls.def("get_immutable_key",
               [](const c_box &self) {
                   return pyg::Tuple<coord_t, coord_t, coord_t, coord_t>::make_tuple(
                       xl(self), yl(self), xh(self), yh(self));
               },
               "Returns a tuple representing this box.");
}
