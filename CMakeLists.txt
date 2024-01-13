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
project(SamlibInfo)

set(CMAKE_CXX_STANDARD 20)

#add_compile_definitions(DEBUG)

add_link_options("-lsqlite3" "-lcurl" "-liconv")  # try to replace by pure CMake construction
include_directories("include")

add_executable(
        SamlibInfo
        src/main.cpp
        src/db.cpp
        include/db.h
        src/http.cpp
        include/http.h
        src/parser.cpp
        include/parser.h
        src/miner.cpp
        include/miner.h
        include/tools.h
        src/tools.cpp
        include/errors.h
        src/logger.cpp
        include/logger.h
        src/agent.cpp
        include/agent.h
        src/fs.cpp
        include/fs.h
)
