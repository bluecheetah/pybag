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

from typing import List

import pytest

from pybag.core import get_cdba_name_bits

name_data = [
    ('foo<2:0>', ['foo<2>', 'foo<1>', 'foo<0>']),
    ('a,b', ['a', 'b']),
    ('foo<1:0>,bar', ['foo<1>', 'foo<0>', 'bar']),
    ('<*2>(a,b)', ['a', 'b', 'a', 'b']),
]


@pytest.mark.parametrize("test_str, expect", name_data)
def test_get_cdba_name_bits(test_str: str, expect: List[str]):
    ans = get_cdba_name_bits(test_str)
    assert ans == expect
