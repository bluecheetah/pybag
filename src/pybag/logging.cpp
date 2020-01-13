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

#include <memory>
#include <string>

#include <cbag/logging/logging.h>

#include <pybag/enum_conv.h>
#include <pybag/logging.h>

void bind_logging(py::module &m) {
    using log_cls = cbag::file_logger;

    auto py_cls = py::class_<log_cls>(m, "FileLogger");
    py_cls.doc() = "A file logger using C++ spdlog library.";
    py_cls.def(py::init([](const std::string &name, const std::string &log_name,
                           int stdout_level = 3) { return log_cls(name, log_name, stdout_level); }),
               "Create a new file logger.", py::arg("name"), py::arg("log_fname"),
               py::arg("stdout_level"));
    py_cls.def_property_readonly(
        "level", [](const log_cls &obj) { return pybag::util::code_to_log_level(obj.level()); },
        "the current logging level.");
    py_cls.def_property_readonly(
        "flush_level",
        [](const log_cls &obj) { return pybag::util::code_to_log_level(obj.flush_level()); },
        "the current flushing level.");
    py_cls.def_property_readonly("log_basename", &log_cls::log_basename, "log file basename.");
    py_cls.def("trace", &log_cls::trace, "log trace message.", py::arg("msg"));
    py_cls.def("debug", &log_cls::debug, "log debug message.", py::arg("msg"));
    py_cls.def("info", &log_cls::info, "log info message.", py::arg("msg"));
    py_cls.def("warn", &log_cls::warn, "log warn message.", py::arg("msg"));
    py_cls.def("error", &log_cls::error, "log error message.", py::arg("msg"));
    py_cls.def("critical", &log_cls::critical, "log critical message.", py::arg("msg"));
    py_cls.def("log", &log_cls::log, "log message with level.", py::arg("level"), py::arg("msg"));
    py_cls.def("flush", &log_cls::flush, "flush the logger.");
    py_cls.def("set_level", &log_cls::set_level, "set logging level.", py::arg("level"));
    py_cls.def("flush_on", &log_cls::flush_on, "set flush level.", py::arg("level"));

    m.def("get_bag_logger", []() { return log_cls("bag", "bag.log", 3); }, "Get the BAG logger.");
}
