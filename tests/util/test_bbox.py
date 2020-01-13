# SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
# Copyright 2018 Regents of the University of California
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

from typing import Tuple

import pytest

from pybag.core import BBox, Transform
from pybag.enum import Orientation, Orient2D

# test data for PyDisjointIntervals
bbox_data = [
    (0, 0, 0, 0, False, True),
    (0, 0, -1, -1, False, False),
    (2, 4, 2, 7, False, True),
    (2, 4, 2, -1, False, False),
    (1, 4, 4, 4, False, True),
    (1, 4, 4, 3, False, False),
    (-2, -3, 6, 12, True, True),
    (0, 0, 3, 5, True, True),
]

transform_data = [
    ((1, 1, 3, 6), 2, -4, "R0", (3, -3, 5, 2)),
    ((0, 0, 3, 6), 0, 0, "R90", (-6, 0, 0, 3)),
    ((0, 0, 3, 6), 1, 1, "R90", (-5, 1, 1, 4)),
]

move_by_orient_data = [
    ((550, 80, 619, 2992), Orient2D.y, 0, 128, (678, 80, 747, 2992)),
]


def test_invalid_bbox():
    ans = BBox.get_invalid_bbox()
    assert ans.is_valid() is False


@pytest.mark.parametrize("xl, yl, xh, yh, physical, valid", bbox_data)
def test_properties(xl, yl, xh, yh, physical, valid):
    ans = BBox(xl, yl, xh, yh)
    assert ans.xl == xl
    assert ans.yl == yl
    assert ans.xh == xh
    assert ans.yh == yh
    assert ans.xm == (xl + xh) // 2
    assert ans.ym == (yl + yh) // 2
    assert ans.w == xh - xl
    assert ans.h == yh - yl
    assert ans.get_immutable_key() == (xl, yl, xh, yh)
    assert ans.get_dim(Orient2D.x) == ans.w
    assert ans.get_dim(Orient2D.y) == ans.h
    assert ans.get_interval(Orient2D.x) == (xl, xh)
    assert ans.get_interval(Orient2D.y) == (yl, yh)

    """
    assert ans.left_unit == xl
    assert ans.bottom_unit == yl
    assert ans.right_unit == xh
    assert ans.top_unit == yh
    assert ans.xc_unit == (xl + xh) // 2
    assert ans.yc_unit == (yl + yh) // 2
    assert ans.width_unit == xh - xl
    assert ans.height_unit == yh - yl
    """


@pytest.mark.parametrize("xl, yl, xh, yh, physical, valid", bbox_data)
def test_physical_valid(xl, yl, xh, yh, physical, valid):
    ans = BBox(xl, yl, xh, yh)
    assert ans.is_physical() == physical
    assert ans.is_valid() == valid

    # check orientation based constructor works
    b1 = BBox(Orient2D.x, xl, xh, yl, yh)
    b2 = BBox(Orient2D.y, yl, yh, xl, xh)
    assert b1 == ans
    assert b2 == ans


@pytest.mark.parametrize("box0, dx, dy, orient, box1", transform_data)
def test_transform(box0, dx, dy, orient, box1):
    xform = Transform(dx, dy, Orientation[orient])
    a = BBox(box0[0], box0[1], box0[2], box0[3])
    ans = BBox(box1[0], box1[1], box1[2], box1[3])

    b = a.get_transform(xform)
    assert b == ans
    assert b is not a
    c = a.transform(xform)
    assert a == ans
    assert c is a


def test_merge_invalid():
    a = BBox(0, 0, 2, 3)
    b = BBox(100, 103, 200, 102)

    a2 = a.get_merge(b)
    a3 = b.get_merge(a)
    assert a2 == a
    assert a3 == a
    assert a2 is not a
    assert a3 is not a

    a2 = a.merge(b)
    assert a2 is a
    b2 = b.merge(a)
    assert b2 is b
    assert b2 == a


@pytest.mark.parametrize("coords, orient, dt, dp, ecoords", move_by_orient_data)
def test_move_by_orient(coords: Tuple[int, int, int, int], orient: Orient2D, dt: int, dp: int,
                        ecoords: Tuple[int, int, int, int]) -> None:
    box = BBox(coords[0], coords[1], coords[2], coords[3])
    expect = BBox(ecoords[0], ecoords[1], ecoords[2], ecoords[3])

    box.move_by_orient(orient, dt, dp)
    assert box == expect


@pytest.mark.parametrize("coords, orient, dt, dp, ecoords", move_by_orient_data)
def test_get_move_by_orient(coords: Tuple[int, int, int, int], orient: Orient2D, dt: int, dp: int,
                            ecoords: Tuple[int, int, int, int]) -> None:
    box_old = BBox(coords[0], coords[1], coords[2], coords[3])
    box = BBox(coords[0], coords[1], coords[2], coords[3])
    expect = BBox(ecoords[0], ecoords[1], ecoords[2], ecoords[3])

    box_new = box.get_move_by_orient(orient, dt, dp)
    assert box == box_old
    assert box_new == expect
