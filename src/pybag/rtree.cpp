// SPDX-License-Identifier: Apache-2.0
/*
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

#include <unordered_map>
#include <utility>

#include <boost/geometry.hpp>

#include <cbag/common/typedefs.h>
#include <cbag/polygon/boost_adapt.h>

#include <pybind11_generics/iterator.h>

#include <pybag/rtree.h>

namespace pybag {
namespace util {

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

class rtree {
  public:
    using coordinate_type = cbag::coord_t;
    using point_type = bg::model::point<coordinate_type, 2, bg::cs::cartesian>;
    using box_type = cbag::polygon::rectangle_data<coordinate_type>;
    using tree_value_type = std::pair<box_type, std::size_t>;
    using tree_type = bgi::rtree<tree_value_type, bgi::quadratic<32, 16>>;
    using const_iterator = typename tree_type::const_iterator;
    using const_query_iterator = typename tree_type::const_query_iterator;

  private:
    tree_type index_;
    std::unordered_map<std::size_t, std::pair<box_type, py::object>> val_map_;
    std::size_t cnt_ = 0;

  public:
    rtree() = default;

    bool empty() const noexcept { return val_map_.empty(); }

    box_type get_bbox() const {
        auto tmp = index_.bounds();
        return {tmp.min_corner().template get<0>(), tmp.min_corner().template get<1>(),
                tmp.max_corner().template get<0>(), tmp.max_corner().template get<1>()};
    }

    py::object get_value(std::size_t id) const {
        auto iter = val_map_.find(id);
        if (iter == val_map_.end()) {
            throw std::out_of_range("Cannot find id in rtree.");
        }
        return iter->second.second;
    }

    const_iterator begin() const { return index_.begin(); }
    const_iterator end() const { return index_.end(); }

    const_query_iterator intersect(const box_type &box) const {
        return index_.qbegin(bgi::intersects(box));
    }
    const_query_iterator overlap(const box_type &box) const {
        return index_.qbegin(bgi::overlaps(box));
    }
    const_query_iterator qend() const { return index_.qend(); }

    std::size_t insert(py::object &&obj, box_type &&box) {
        auto ans = cnt_;
        index_.insert(tree_value_type(box, ans));
        val_map_.emplace(ans, std::make_pair(std::move(box), std::move(obj)));
        ++cnt_;
        return ans;
    }

    py::object pop(std::size_t id) {
        auto iter = val_map_.find(id);
        if (iter == val_map_.end()) {
            throw std::out_of_range("Cannot find id in rtree.");
        }
        auto ans = iter->second.second;
        auto val = tree_value_type(iter->second.first, id);
        index_.remove(val);
        val_map_.erase(iter);
        return ans;
    }
};

} // namespace util
} // namespace pybag

namespace pu = pybag::util;
namespace pyg = pybind11_generics;

void bind_rtree(py::module &m) {
    pyg::declare_iterator<pu::rtree::const_iterator>();
    pyg::declare_iterator<pu::rtree::const_query_iterator>();

    auto py_cls = py::class_<pu::rtree>(m, "RTree");
    py_cls.def(py::init<>(), "Create an empty RTree.");
    py_cls.def("__bool__", [](const pu::rtree &self) { return !self.empty(); },
               "True if this object is not empty.");
    py_cls.def("__iter__",
               [](const pu::rtree &self) { return pyg::make_iterator(self.begin(), self.end()); },
               "Iterates over all items in this Rtree.");
    py_cls.def("__getitem__",
               [](const pu::rtree &self, std::size_t id) { return self.get_value(id); },
               "Returns the value corresponding to the given ID.", py::arg("obj_id"));
    py_cls.def("pop", [](pu::rtree &self, std::size_t id) { return self.pop(id); },
               "Removes the given ID from this RTree and returns the associated value.",
               py::arg("obj_id"));
    py_cls.def_property_readonly("bound_box", &pu::rtree::get_bbox, "The overall bounding box.");
    py_cls.def(
        "intersect_iter",
        [](const pu::rtree &self, const pu::rtree::box_type &box) {
            return pyg::make_iterator(self.intersect(box), self.qend());
        },
        "Returns an iterator over all objects that intersects the given box (includes touches).",
        py::arg("box"));
    py_cls.def(
        "overlap_iter",
        [](const pu::rtree &self, const pu::rtree::box_type &box) {
            return pyg::make_iterator(self.overlap(box), self.qend());
        },
        "Returns an iterator over all objects that overlaps the given box (excludes touches).",
        py::arg("box"));
    py_cls.def("insert",
               [](pu::rtree &self, py::object obj, pu::rtree::box_type box) {
                   return self.insert(std::move(obj), std::move(box));
               },
               "Insert given object into RTree.", py::arg("obj"), py::arg("box"));
}
