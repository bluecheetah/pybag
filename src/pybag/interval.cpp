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

#include <array>
#include <memory>
#include <utility>

#include <pybind11/pybind11.h>

#include <pybind11_generics/any.h>

#include <cbag/util/interval.h>

#include <pybind11_generics/iterator.h>
#include <pybind11_generics/optional.h>
#include <pybind11_generics/tuple.h>

#include <pybag/interval.h>

namespace pyg = pybind11_generics;

using coord_type = cbag::offset_t;
using py_intv_type = std::pair<coord_type, coord_type>;
using py_interval = std::pair<py_intv_type, pyg::Any>;
using c_dis_intvs = cbag::util::disjoint_intvs<py_interval>;

namespace cbag {
namespace util {
namespace traits {

template <> struct coordinate_type<py_intv_type> {
    using coord_type = cbag::offset_t;
    using len_type = cbag::offset_t;
};
template <> struct interval<py_intv_type> {
    using intv_type = py_intv_type;
    using coordinate_type = coordinate_type<intv_type>::coord_type;

    static intv_type &intv(intv_type &i) { return i; }
    static const intv_type &intv(const intv_type &i) { return i; }
    static coordinate_type start(const intv_type &i) { return i.first; }
    static coordinate_type stop(const intv_type &i) { return i.second; }
    static void set_start(intv_type &i, coordinate_type val) { i.first = val; }
    static void set_stop(intv_type &i, coordinate_type val) { i.second = val; }
    static intv_type construct(coordinate_type start, coordinate_type stop) {
        return {start, stop};
    }
};

template <> struct coordinate_type<py_interval> {
    using coord_type = cbag::offset_t;
    using len_type = cbag::offset_t;
};
template <> struct interval<py_interval> {
    using coordinate_type = coordinate_type<py_interval>::coord_type;
    using intv_type = py_intv_type;

    static intv_type &intv(py_interval &i) { return i.first; }
    static const intv_type &intv(const py_interval &i) { return i.first; }
    static coordinate_type start(const py_interval &i) { return i.first.first; }
    static coordinate_type stop(const py_interval &i) { return i.first.second; }
    static void set_start(py_interval &i, coordinate_type val) { i.first.first = val; }
    static void set_stop(py_interval &i, coordinate_type val) { i.first.second = val; }
    static py_interval construct(coordinate_type start, coordinate_type stop) {
        return std::make_pair(intv_type{start, stop}, py::none());
    }
};

} // namespace traits
} // namespace util
} // namespace cbag

namespace pybag {
namespace util {

class const_val_iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = const pyg::Any;
    using difference_type = c_dis_intvs::const_intv_iterator::difference_type;
    using pointer = value_type *;
    using reference = value_type &;

  private:
    c_dis_intvs::const_iterator iter_;

  public:
    const_val_iterator() = default;
    const_val_iterator(c_dis_intvs::const_iterator val) : iter_(std::move(val)) {}

    bool operator==(const const_val_iterator &other) const { return iter_ == other.iter_; }
    bool operator!=(const const_val_iterator &other) const { return iter_ != other.iter_; }

    reference operator*() const { return iter_->second; }
    pointer operator->() const { return &operator*(); }

    const_val_iterator &operator++() {
        ++iter_;
        return *this;
    }
    const_val_iterator operator++(int) {
        const_val_iterator ans(iter_);
        operator++();
        return ans;
    }
};

pyg::PyIterator<py_intv_type> py_intv_iterator(const c_dis_intvs &self) {
    return pyg::make_iterator(self.intv_begin(), self.intv_end());
}
pyg::PyIterator<py_interval> py_item_iterator(const c_dis_intvs &self) {
    return pyg::make_iterator(self.begin(), self.end());
}
pyg::PyIterator<pyg::Any> py_val_iterator(const c_dis_intvs &self) {
    return pyg::make_iterator(const_val_iterator(self.begin()), const_val_iterator(self.end()));
}
pyg::PyIterator<py_intv_type> py_ovl_intv_iterator(const c_dis_intvs &self,
                                                   const py_intv_type &key) {
    auto iter_pair = self.overlap_range(key);
    return pyg::make_iterator(c_dis_intvs::const_intv_iterator(iter_pair.first),
                              c_dis_intvs::const_intv_iterator(iter_pair.second));
}
pyg::PyIterator<py_interval> py_ovl_item_iterator(const c_dis_intvs &self,
                                                  const py_intv_type &key) {
    auto iter_pair = self.overlap_range(key);
    return pyg::make_iterator(iter_pair.first, iter_pair.second);
}
pyg::PyIterator<pyg::Any> py_ovl_val_iterator(const c_dis_intvs &self, const py_intv_type &key) {
    auto iter_pair = self.overlap_range(key);
    return pyg::make_iterator(const_val_iterator(iter_pair.first),
                              const_val_iterator(iter_pair.second));
}

pyg::Optional<pyg::Tuple<pyg::Tuple<int, int>, pyg::Any>>
get_first_overlap_item(const c_dis_intvs &self, const py_intv_type &key) {
    auto iter_pair = self.overlap_range(key);
    if (iter_pair.first == iter_pair.second)
        return py::none();
    return py::cast(*(iter_pair.first));
}

bool add(c_dis_intvs &self, pyg::Tuple<int, int> intv, pyg::Any val = py::none(),
         bool merge = false, bool abut = false, bool check_only = false) {
    return self.emplace(merge, abut, check_only,
                        py_intv_type{intv[0].cast<py::int_>(), intv[1].cast<py::int_>()}, val);
}

} // namespace util
} // namespace pybag

namespace pu = pybag::util;

void bind_interval(py::module &m) {
    pyg::declare_iterator<c_dis_intvs::const_iterator>();
    pyg::declare_iterator<c_dis_intvs::const_intv_iterator>();
    pyg::declare_iterator<pu::const_val_iterator>();

    // add interval class
    auto py_dis_intvs = py::class_<c_dis_intvs>(m, "PyDisjointIntervals");
    py_dis_intvs.doc() = "A class that keeps track of disjoint intervals.";

    py_dis_intvs.def(py::init<>(), "Construct an empty PyDisjointIntervals set.");
    py_dis_intvs.def_property_readonly("start", &c_dis_intvs::start,
                                       "The start coordinate of first interval.");
    py_dis_intvs.def_property_readonly("stop", &c_dis_intvs::stop,
                                       "The stop coordinate of last interval.");
    py_dis_intvs.def("__contains__", &c_dis_intvs::contains<py_intv_type>,
                     "Returns True if given interval is in this object.", py::arg("key"));
    py_dis_intvs.def("__iter__", &pu::py_intv_iterator, py::keep_alive<0, 1>(),
                     "Iterates through intervals.");
    py_dis_intvs.def("__len__", &c_dis_intvs::size, "Returns number of intervals.");
    py_dis_intvs.def("__bool__", [](const c_dis_intvs &self) { return !self.empty(); },
                     "Returns True if it contains at least one interval.");
    py_dis_intvs.def("__repr__", &c_dis_intvs::to_string,
                     "Returns a string representation of this interval.");
    py_dis_intvs.def("overlaps", &c_dis_intvs::overlaps<py_intv_type>,
                     "Returns True if given interval overlaps this object.", py::arg("key"));
    py_dis_intvs.def(
        "covers", &c_dis_intvs::covers<py_intv_type>,
        "Returns True if given interval is covered by a single interval in this object.",
        py::arg("key"));
    py_dis_intvs.def(
        "covers",
        [](const c_dis_intvs &self, coord_type val) {
            return self.covers(std::make_pair(val, val + 1));
        },
        "Returns True if given integer is covered by a single interval in this object.",
        py::arg("key"));
    py_dis_intvs.def(
        "get_next", &c_dis_intvs::get_next,
        "Returns the value greater than or equal to the given value contained in this object.",
        py::arg("val"), py::arg("even") = true);
    py_dis_intvs.def(
        "get_prev", &c_dis_intvs::get_prev,
        "Returns the value less than or equal to the given value contained in this object.",
        py::arg("val"), py::arg("even") = true);
    py_dis_intvs.def("items", &pu::py_item_iterator, py::keep_alive<0, 1>(),
                     "Iterates through intervals and values.");
    py_dis_intvs.def("intervals", &pu::py_intv_iterator, py::keep_alive<0, 1>(),
                     "Iterates through intervals.");
    py_dis_intvs.def("values", &pu::py_val_iterator, py::keep_alive<0, 1>(),
                     "Iterates through values.");
    py_dis_intvs.def("overlap_items", &pu::py_ovl_item_iterator, py::keep_alive<0, 1>(),
                     "Iterates through overlapping intervals and values.", py::arg("key"));
    py_dis_intvs.def("overlap_intervals", &pu::py_ovl_intv_iterator, py::keep_alive<0, 1>(),
                     "Iterates through overlapping intervals.", py::arg("key"));
    py_dis_intvs.def("overlap_values", &pu::py_ovl_val_iterator, py::keep_alive<0, 1>(),
                     "Iterates through overlapping values.", py::arg("key"));
    py_dis_intvs.def("get_first_overlap_item", &pu::get_first_overlap_item,
                     "Returns the first overlap interval and value.", py::arg("key"));
    py_dis_intvs.def("get_copy", &c_dis_intvs::get_copy, "Returns a copy of this object.");
    py_dis_intvs.def("get_intersection", &c_dis_intvs::get_intersection,
                     "Returns the intersection.", py::arg("other"));
    py_dis_intvs.def("get_complement", &c_dis_intvs::get_complement<py_intv_type>,
                     "Returns the complement.", py::arg("total_intv"));
    py_dis_intvs.def("get_transform", &c_dis_intvs::get_transform,
                     "Returns the transformed intervals.", py::arg("scale") = 1,
                     py::arg("shift") = 0);
    py_dis_intvs.def("remove", &c_dis_intvs::remove<py_intv_type>, "Removes the given interval.",
                     py::arg("key"));
    py_dis_intvs.def("remove_overlaps", &c_dis_intvs::remove_overlaps<py_intv_type>,
                     "Removes overlapping intervals.", py::arg("key"));
    py_dis_intvs.def("add", &pu::add, "add the given interval.", py::arg("intv"),
                     py::arg("val") = py::none(), py::arg("merge") = false, py::arg("abut") = false,
                     py::arg("check_only") = false);
    py_dis_intvs.def("subtract", &c_dis_intvs::subtract<py_intv_type>,
                     "Subtracts the given interval.", py::arg("key"));
}
