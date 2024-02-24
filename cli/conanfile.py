#
# Copyright 2024 Yurii Havenchuk.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import os

from conan import ConanFile
from conan.tools.cmake import CMake


class MyProjectConan(ConanFile):
    name = 'cli'
    version = '1.0'
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'CMakeToolchain'
    requires = 'core/0.1', 'boost/1.84.0', 'libiconv/1.17'  # `conan` needs `libiconv` for the `boost_locale`
    exports_sources = 'CMakeLists.txt', 'src/*'
    default_options = {'*:shared': True}

    def package_info(self):
        self.settings.compiler.cppstd = '20'

    def layout(self):
        current_directory = os.path.abspath(os.path.dirname(__file__))
        build_directory = os.path.join(current_directory, '..', f'cmake-build-{str(self.settings.build_type).lower()}')

        self.folders.source = current_directory
        self.folders.build = build_directory
        self.folders.generators = os.path.join(build_directory, 'conan')

    def build(self):
        cmake = CMake(self)
        cmake.configure(cli_args=['-G', 'Ninja'])
        cmake.build()
