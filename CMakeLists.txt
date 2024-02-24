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


cmake_minimum_required(VERSION 3.27)
set(APP_NAME SamlibInfo)
project(${APP_NAME})

set(CMAKE_CXX_STANDARD 20)

if(DEFINED CONAN_COMMAND AND NOT EXISTS "${CMAKE_SOURCE_DIR}/conan_provider.cmake")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/develop2/conan_provider.cmake" "${CMAKE_SOURCE_DIR}/conan_provider.cmake")
endif()

add_subdirectory(core)
add_subdirectory(cli)
