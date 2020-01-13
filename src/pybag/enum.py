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

"""This package contains various enums used by the C++ extension."""

from __future__ import annotations

from enum import IntEnum


class DesignOutput(IntEnum):
    LAYOUT = 0
    GDS = 1
    SCHEMATIC = 2
    YAML = 3
    CDL = 4
    VERILOG = 5
    SYSVERILOG = 6
    SPECTRE = 7
    OASIS = 8

    @property
    def extension(self) -> str:
        if self is DesignOutput.GDS:
            return 'gds'
        elif self is DesignOutput.YAML:
            return 'yaml'
        elif self is DesignOutput.CDL:
            return 'cdl'
        elif self is DesignOutput.VERILOG:
            return 'v'
        elif self is DesignOutput.SYSVERILOG:
            return 'sv'
        elif self is DesignOutput.SPECTRE:
            return 'scs'
        elif self is DesignOutput.OASIS:
            return 'oasis'
        else:
            raise ValueError(f'Unsupported output type: {self.name}')

    @property
    def is_model(self) -> bool:
        return self is DesignOutput.SYSVERILOG or self is DesignOutput.VERILOG

    @property
    def is_netlist(self) -> bool:
        return self is DesignOutput.CDL or self is DesignOutput.SPECTRE

    @property
    def fallback_model_type(self) -> DesignOutput:
        if self is DesignOutput.SYSVERILOG:
            return DesignOutput.VERILOG
        else:
            return self

    @property
    def is_layout(self) -> bool:
        return self is DesignOutput.LAYOUT or self is DesignOutput.GDS or self is DesignOutput.OASIS


class TermType(IntEnum):
    input = 0
    output = 1
    inout = 2


class SigType(IntEnum):
    signal = 0
    power = 1
    ground = 2
    clock = 3
    tieOff = 4
    tieHi = 5
    tieLo = 6
    analog = 7
    scan = 8
    reset = 9


class RoundMode(IntEnum):
    LESS = -2
    LESS_EQ = -1
    NEAREST = 0
    GREATER_EQ = 1
    GREATER = 2
    NONE = 3


class MinLenMode(IntEnum):
    LOWER = -1
    MIDDLE = 0
    UPPER = 1
    NONE = 2


class PinMode(IntEnum):
    LOWER = -1
    ALL = 0
    UPPER = 1
    MIDDLE = 2


class Orientation(IntEnum):
    R0 = 0
    MY = 1
    MX = 2
    R180 = 3
    MXR90 = 4
    R90 = 5
    R270 = 6
    MYR90 = 7

    def flip_lr(self) -> Orientation:
        return Orientation(self.value ^ 0b001)

    def flip_ud(self) -> Orientation:
        return Orientation(self.value ^ 0b010)


class Orient2D(IntEnum):
    x = 0
    y = 1

    def perpendicular(self) -> Orient2D:
        return Orient2D(1 - self.value)


class Direction(IntEnum):
    LOWER = 0
    UPPER = 1

    def flip(self) -> Direction:
        return Direction(1 - self.value)


class Direction2D(IntEnum):
    WEST = 0
    SOUTH = 1
    EAST = 2
    NORTH = 3

    @property
    def is_vertical(self) -> bool:
        return (self.value & 1) == 1

    def flip(self) -> Direction2D:
        return Direction2D(self.value ^ 2)


class PathStyle(IntEnum):
    truncate = 0
    extend = 1
    round = 2
    triangle = 3


class BlockageType(IntEnum):
    routing = 0
    via = 1
    placement = 2
    wiring = 3
    fill = 4
    slot = 5
    pin = 6
    feed_thru = 7
    screen = 8


class BoundaryType(IntEnum):
    PR = 0
    snap = 1


class GeometryMode(IntEnum):
    POLY_90 = 0
    POLY_45 = 1
    POLY = 2


class SpaceQueryMode(IntEnum):
    DIFF_COLOR = 0
    SAME_COLOR = 1
    LINE_END = 2


class LogLevel(IntEnum):
    TRACE = 0
    DEBUG = 1
    INFO = 2
    WARN = 3
    ERROR = 4
    CRITICAL = 5
    OFF = 6


class SupplyWrapMode(IntEnum):
    NONE = 0
    TOP = 1
