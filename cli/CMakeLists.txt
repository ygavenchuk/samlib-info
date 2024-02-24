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

set(CMAKE_CXX_STANDARD 20)

find_package(Iconv REQUIRED)  # `conan` requires this library for the `boost_locale`
find_package(Boost 1.62.0 REQUIRED COMPONENTS program_options)
find_package(Boost 1.62.0 REQUIRED COMPONENTS locale)

message(STATUS "Boost include dirs at ${Boost_INCLUDE_DIRS}")


include_directories(
        ${Boost_INCLUDE_DIRS}
        ${Iconv_INCLUDE_DIR}
        "../core/include"
)

add_executable(
        ${APP_NAME}
        src/cli.cpp
)

target_link_libraries(
        ${APP_NAME}
        PRIVATE ${Iconv_LIBRARY}
        PRIVATE ${Boost_PROGRAM_OPTIONS_LIBRARY}
        PRIVATE ${Boost_LOCALE_LIBRARY} # required for CLI
        PRIVATE "samlib-info"
)
