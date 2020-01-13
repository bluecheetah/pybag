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
import re
import sys
import platform
import subprocess
from pathlib import Path

from distutils.version import LooseVersion
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

pkg_name = 'pybag'


class CMakePyBind11Extension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = str(Path(sourcedir).resolve())


class CMakePyBind11Build(build_ext):
    user_options = build_ext.user_options
    user_options.append(('compiler-launcher=', None, "specify compiler launcher program"))
    user_options.append(('build-type=', None, "CMake build type"))

    def initialize_options(self):
        build_ext.initialize_options(self)
        # noinspection PyAttributeOutsideInit
        self.compiler_launcher = ''
        # noinspection PyAttributeOutsideInit
        self.build_type = 'Debug'

    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError('CMake must be installed to build the following extensions: ' +
                               ', '.join(e.name for e in self.extensions))

        if platform.system() == 'Windows':
            cmake_version = LooseVersion(re.search(r'version\s*([\d.]+)',
                                                   out.decode()).group(1))
            if cmake_version < '3.1.0':
                raise RuntimeError('CMake >= 3.1.0 is required on Windows')

        for ext in self.extensions:
            self.build_extension(ext)

    def _get_num_workers(self):
        workers = self.parallel
        if self.parallel == 0:
            workers = os.cpu_count()  # may return None
            if workers is None:
                workers = 1
            else:
                workers = workers // 2

        return workers

    def build_extension(self, ext):
        # setup CMake initialization and build commands
        out_dir = str(Path(self.build_lib).resolve() / pkg_name)
        init_cmd = [
            'cmake',
            '-H.',
            f'-B{self.build_temp}',
            f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={out_dir}',
            f'-DPYTHON_EXECUTABLE={sys.executable}',
            f'-DCMAKE_BUILD_TYPE={self.build_type}',
        ]
        build_cmd = ['cmake', '--build', self.build_temp, '--']

        # handle compiler launcher
        print('comp_launcher =', self.compiler_launcher)
        if self.compiler_launcher:
            init_cmd.append('-DCMAKE_CXX_COMPILER_LAUNCHER=' + self.compiler_launcher)

        # handle Windows CMake arguments
        if platform.system() == "Windows":
            if sys.maxsize > 2 ** 32:
                init_cmd.append('-A')
                init_cmd.append('x64')
            build_cmd.append('/m')

        # set up parallel build arguments
        num_workers = self._get_num_workers()
        print(f'parallel={num_workers}')
        build_cmd.append(f'-j{num_workers}')

        # run CMake
        Path(self.build_temp).mkdir(parents=True, exist_ok=True)
        print(f'CMake command: {" ".join(init_cmd)}')
        print(f'Build command: {" ".join(build_cmd)}')
        subprocess.check_call(init_cmd)
        subprocess.check_call(build_cmd)
        # generate stubs
        subprocess.check_call(['./gen_stubs.sh'])

        print()  # Add an empty line for cleaner output


setup(
    name=pkg_name,
    version='0.2',
    author='Eric Chang',
    author_email='info@bcanalog.com',
    description='Python wrappers of cbag library using pybind11',
    license='Apache-2.0',
    install_requires=[],
    setup_requires=[],
    tests_require=[
        'pytest',
        'pytest-xdist',
    ],
    packages=[
        pkg_name,
    ],
    package_dir={'': 'src'},
    ext_modules=[CMakePyBind11Extension('all')],
    cmdclass={'build_ext': CMakePyBind11Build},
    zip_safe=False,
)
