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

import os

import pkg_resources

from pybag.core import PySchCellView


def build_cv(fname):
    yaml_file = pkg_resources.resource_filename(__name__, os.path.join('data', fname))

    return PySchCellView(yaml_file)


def test_constructor():
    """Check PySchCellView is constructed properly."""
    fname = 'inv.yaml'

    cv = build_cv(fname)
    assert list(cv.terminals()) == [('VDD', 0), ('VSS', 0), ('in', 0), ('out', 1)]


def test_array_instance():
    """Check array_instance works."""
    fname = 'inv.yaml'

    cv = build_cv(fname)

    conn1 = [('B', 'foo'), ('D', 'bar')]
    conn2 = [('G', 'baz'), ('S', 'ok')]

    name_conn_range = [('XFOO1', conn1), ('XFOO2', iter(conn2))]

    cv.array_instance('XP', 0, 0, name_conn_range)

    # make sure old instance is deleted
    assert cv.get_inst_ref('XP') is None
    inst1 = cv.get_inst_ref('XFOO1')
    inst2 = cv.get_inst_ref('XFOO2')
    # make sure new instance exists
    assert inst1 is not None
    assert inst2 is not None

    # make sure new instance properties are correct
    for inst, conn in ((inst1, conn1), (inst2, conn2)):
        assert inst.lib_name == 'BAG_prim'
        assert inst.cell_name == 'pmos4_standard'
        assert inst.is_primitive
        for term, net in conn:
            assert inst.get_connection(term) == net


def test_to_yaml():
    """Check yaml conversion method works."""
    fname = 'inv.yaml'

    cv = build_cv(fname)

    content = cv.to_yaml()
    yaml_file = pkg_resources.resource_filename(__name__, os.path.join('data', fname))
    with open(yaml_file, 'r') as f:
        file_content = f.read()

    assert content == file_content
