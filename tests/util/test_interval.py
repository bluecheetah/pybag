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

import sys

import pytest

from pybag.core import PyDisjointIntervals

# test data for PyDisjointIntervals
intv_vals_data = [
    [],
    [((1, 2), 'A'), ((3, 5), 'B')],
    [((1, 2), 1), ((4, 5), 2), ((6, 8), 3)],
]

ovl_data = [
    ([], (1, 2), None),
    ([((1, 2), 'A'), ((3, 5), 'B')], (3, 5), ((3, 5), 'B')),
    ([((1, 2), 1), ((4, 7), 2), ((10, 12), 3)], (6, 8), ((4, 7), 2)),
    ([((1, 2), 1), ((4, 7), 2), ((10, 12), 3)], (6, 11), ((4, 7), 2)),
]


def make_dis_intvs(intv_vals):
    ans = PyDisjointIntervals()
    for intv, val in intv_vals:
        ans.add(intv, val=val)
    return ans


@pytest.mark.parametrize("intv_vals", intv_vals_data)
def test_constructor_refcount(intv_vals):
    """Check class increment reference count of value objects."""
    bc_list = [sys.getrefcount(v) for _, v in intv_vals]
    # keep reference to PyDisjointInterval to avoid garbage collection
    # noinspection PyUnusedLocal
    dis_intvs = make_dis_intvs(intv_vals)
    ac_list = [sys.getrefcount(v) for _, v in intv_vals]
    for bc, ac in zip(bc_list, ac_list):
        assert ac == bc + 1


@pytest.mark.parametrize("intv_vals", intv_vals_data)
def test_destructor_refcount(intv_vals):
    """Check class decrement reference count of value objects."""
    dis_intvs = make_dis_intvs(intv_vals)
    bc_list = [sys.getrefcount(v) for _, v in intv_vals]
    del dis_intvs
    ac_list = [sys.getrefcount(v) for _, v in intv_vals]
    for bc, ac in zip(bc_list, ac_list):
        assert ac == bc - 1


@pytest.mark.parametrize("intv_vals", intv_vals_data)
def test_contains(intv_vals):
    dis_intvs = make_dis_intvs(intv_vals)
    for intv, _ in intv_vals:
        assert intv in dis_intvs
        assert (intv[0], intv[1] + 1) not in dis_intvs


@pytest.mark.parametrize("intv_vals", intv_vals_data)
def test_iter(intv_vals):
    dis_intvs = make_dis_intvs(intv_vals)
    for (a, _), b in zip(intv_vals, dis_intvs):
        assert a == b


@pytest.mark.parametrize("intv_vals", intv_vals_data)
def test_len(intv_vals):
    dis_intvs = make_dis_intvs(intv_vals)
    assert len(intv_vals) == len(dis_intvs)


@pytest.mark.parametrize("intv_vals", intv_vals_data)
def test_get_start(intv_vals):
    dis_intvs = make_dis_intvs(intv_vals)
    if intv_vals:
        assert intv_vals[0][0][0] == dis_intvs.start
    else:
        with pytest.raises(IndexError):
            # noinspection PyStatementEffect
            dis_intvs.start


@pytest.mark.parametrize("intv_vals", intv_vals_data)
def test_get_end(intv_vals):
    dis_intvs = make_dis_intvs(intv_vals)
    if intv_vals:
        assert intv_vals[-1][0][1] == dis_intvs.stop
    else:
        with pytest.raises(IndexError):
            # noinspection PyStatementEffect
            dis_intvs.stop


@pytest.mark.parametrize("intv_vals,key,item", ovl_data)
def test_get_first_overlap_item(intv_vals, key, item):
    """Check getting the first overlap item"""
    dis_intvs = make_dis_intvs(intv_vals)
    assert dis_intvs.get_first_overlap_item(key) == item


def test_abut():
    intvs = PyDisjointIntervals()
    intvs.add((0, 3), 'hi')
    success = intvs.add((3, 5), 'bye', abut=True)
    assert success
    success = intvs.add((5, 7), None)
    assert not success
