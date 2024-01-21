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

find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)
find_package(Iconv REQUIRED)
find_package(Boost 1.62.0 REQUIRED COMPONENTS program_options)
find_package(Boost 1.62.0 REQUIRED COMPONENTS locale)

message(STATUS "Boost include dirs at ${Boost_INCLUDE_DIRS}")

include_directories(
    ${SQLITE3_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${Iconv_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    "include"
)

add_executable(
        ${APP_NAME}
        src/core/db.cpp
        include/core/db.h
        src/core/http.cpp
        include/core/http.h
        src/core/parser.cpp
        include/core/parser.h
        src/core/miner.cpp
        include/core/miner.h
        include/core/tools.h
        src/core/tools.cpp
        include/core/errors.h
        src/core/logger.cpp
        include/core/logger.h
        src/core/agent.cpp
        include/core/agent.h
        src/core/fs.cpp
        include/core/fs.h
        src/cli.cpp
)

target_link_libraries(
        ${APP_NAME}
        ${SQLite3_LIBRARIES}
        ${CURL_LIBRARIES}
        ${Iconv_LIBRARY}
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${Boost_LOCALE_LIBRARY} # required for CLI
)
