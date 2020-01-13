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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <pybind11_generics/list.h>

#include <cbag/enum/design_output.h>
#include <cbag/spirit/util.h>
#include <cbag/util/iterators.h>
#include <cbag/util/name_convert.h>

#include <pybag/name.h>

namespace pyg = pybind11_generics;

namespace pybag {
namespace name {

constexpr auto design_output_default = static_cast<int>(cbag::design_output::SCHEMATIC);

template <typename NS> pyg::List<std::string> _get_name_bits_helper(const std::string &name) {
    auto ans = pyg::List<std::string>();
    auto name_obj = cbag::util::parse_cdba_name(name);
    cbag::spirit::util::get_name_bits(
        name_obj,
        cbag::util::lambda_output_iterator([&ans](const cbag::spirit::ast::name_bit &obj) {
            ans.emplace_back(obj.to_string(false, NS{}));
        }));
    return ans;
}

template <typename NS> py::str _convert_name_bit_helper(const std::string &name) {
    auto name_obj = cbag::util::parse_cdba_name_unit(name);
    if (name_obj.size() != 1) {
        throw std::invalid_argument("The name " + name + " is not a name_bit.");
    }
    return name_obj[0].to_string(false, NS{});
}

pyg::List<std::string> get_cdba_name_bits(const std::string &name,
                                          int design_output_code = design_output_default) {
    auto design_output = static_cast<cbag::design_output>(design_output_code);
    switch (design_output) {
    case cbag::design_output::LAYOUT:
    case cbag::design_output::GDS:
    case cbag::design_output::SCHEMATIC:
    case cbag::design_output::YAML:
    case cbag::design_output::CDL:
        return _get_name_bits_helper<cbag::spirit::namespace_cdba>(name);
    case cbag::design_output::VERILOG:
    case cbag::design_output::SYSVERILOG:
        return _get_name_bits_helper<cbag::spirit::namespace_verilog>(name);
    case cbag::design_output::SPECTRE:
        return _get_name_bits_helper<cbag::spirit::namespace_spectre>(name);
    default:
        throw std::invalid_argument("Unknown design output code: " +
                                    std::to_string(design_output_code));
    }
}

py::str convert_cdba_name_bit(const std::string &name,
                              int design_output_code = design_output_default) {
    auto design_output = static_cast<cbag::design_output>(design_output_code);
    switch (design_output) {
    case cbag::design_output::LAYOUT:
    case cbag::design_output::GDS:
    case cbag::design_output::SCHEMATIC:
    case cbag::design_output::YAML:
    case cbag::design_output::CDL:
        return _convert_name_bit_helper<cbag::spirit::namespace_cdba>(name);
    case cbag::design_output::VERILOG:
    case cbag::design_output::SYSVERILOG:
        return _convert_name_bit_helper<cbag::spirit::namespace_verilog>(name);
    case cbag::design_output::SPECTRE:
        return _convert_name_bit_helper<cbag::spirit::namespace_spectre>(name);
    default:
        throw std::invalid_argument("Unknown design output code: " +
                                    std::to_string(design_output_code));
    }
}

} // namespace name
} // namespace pybag

void bind_name(py::module &m) {
    m.def("get_cdba_name_bits", &pybag::name::get_cdba_name_bits,
          "Get a list of bit names in the given complex name.", py::arg("name"),
          py::arg("design_output_code") = pybag::name::design_output_default);
    m.def("convert_cdba_name_bit", &pybag::name::convert_cdba_name_bit,
          "Convert CDBA name bit to the given design output format.", py::arg("name"),
          py::arg("design_output_code") = pybag::name::design_output_default);
}
