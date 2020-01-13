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

#include <pybag/enum_conv.h>

namespace pybag {
namespace util {

cbag::orient_t get_orient_code(const py::str &orient) {
    // use Python enum class to convert string to int
    py::object orient_cls = py::module::import("pybag.enum").attr("Orientation");
    return orient_cls.attr("__getitem__")(orient).template cast<cbag::orient_t>();
}

pyg::PyOrient code_to_orient(cbag::orient_t code) {
    py::object orient_cls = py::module::import("pybag.enum").attr("Orientation");
    return orient_cls(code);
}

pyg::PyOrient2D code_to_orient_2d(cbag::orientation_2d_t code) {
    py::object orient_cls = py::module::import("pybag.enum").attr("Orient2D");
    return orient_cls(code);
}

pyg::PyLogLevel code_to_log_level(int level) {
    py::object lev_cls = py::module::import("pybag.enum").attr("LogLevel");
    return lev_cls(level);
}

pyg::PySigType code_to_sig_type(cbag::enum_t code) {
    py::object sig_cls = py::module::import("pybag.enum").attr("SigType");
    return sig_cls(code);
}

} // namespace util
} // namespace pybag
