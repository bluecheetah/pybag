# SPDX-License-Identifier: Apache-2.0
# Copyright 2019 Blue Cheetah Analog Design Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import pytest

from pybag.core import RTree, BBox


def test_empty():
    rtree = RTree()

    assert not rtree
    assert not rtree.bound_box.is_valid()

    with pytest.raises(IndexError):
        # noinspection PyStatementEffect
        rtree[0]

    with pytest.raises(IndexError):
        rtree.pop(0)

    objects = [obj for obj in rtree]
    assert not objects
    objects = [obj for obj in rtree.intersect_iter(BBox(0, 0, 1, 1))]
    assert not objects


def test_insert():
    rtree = RTree()

    box_list = [BBox(0, 0, 2, 3), BBox(-2, -3, 0, -1)]
    items = [None, {'hi': 2, 'bye': 'foo'}]

    for obj, box in zip(items, box_list):
        rtree.insert(obj, box)

    for box, obj_id in rtree:
        assert rtree[obj_id] is items[obj_id]
        assert box == box_list[obj_id]


def test_overlap():
    rtree = RTree()

    box_list = [BBox(0, 0, 2, 3), BBox(-2, -3, 0, -1), BBox(10, 10, 20, 15)]
    test_box = BBox(-1, -2, 0, 0)

    for box in box_list:
        rtree.insert(None, box)

    obj_list = list(rtree.overlap_iter(test_box))
    assert len(obj_list) == 1
    assert obj_list[0][0] == box_list[1]


def test_pop():
    rtree = RTree()

    box_list = [BBox(0, 0, 2, 3), BBox(-2, -3, 0, -1), BBox(10, 10, 20, 15)]

    for box in box_list:
        rtree.insert(None, box)

    ans = rtree.pop(1)
    assert ans is None

    results = list(rtree)
    assert len(results) == 2
    for box, obj_id in results:
        assert obj_id != 1
        assert box == box_list[obj_id]
