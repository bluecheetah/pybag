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

from pybag.core import PyRoutingGrid, TrackColoring, PyLayCellView, BBox, PyTrackID


def test_get_intersect(routing_grid: PyRoutingGrid, tr_colors: TrackColoring) -> None:
    cv = PyLayCellView(routing_grid, tr_colors, 'pytest')

    layer = 1
    wire_data = [
        (0, 0, 500),
        (2, 125, 175),
        (5, -200, 150),
        (5, 300, 500),
        (8, 150, 300),
    ]
    expect_idx = {0, 1, 2}
    yl = 100
    yh = 200
    xl = routing_grid.htr_to_coord(layer, 0)
    xh = routing_grid.htr_to_coord(layer, 7)
    bnd_box = BBox(xl, yl, xh, yh)

    expect_set = set()
    for idx, (htr, lower, upper) in enumerate(wire_data):
        tid = PyTrackID(layer, htr, 1, 1, 0)
        cv.add_warr(tid, lower, upper)
        if idx in expect_idx:
            bnds = tid.get_bounds(routing_grid)
            expect_set.add((bnds[0], lower, bnds[1], upper))

    box_list = cv.get_intersect(layer, bnd_box, 0, 0, True)
    ans = {(box.xl, box.yl, box.xh, box.yh) for box in box_list}

    assert ans == expect_set
