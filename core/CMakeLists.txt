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
set(LIBRARY_NAME samlib-info)

set(CMAKE_CXX_STANDARD 20)

find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)
find_package(Iconv REQUIRED)

include_directories(
        ${SQLITE3_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIRS}
        ${Iconv_INCLUDE_DIR}
        "include"
)

add_library(
        ${LIBRARY_NAME} SHARED
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

target_link_libraries(
        ${LIBRARY_NAME} PUBLIC
        PRIVATE ${SQLite3_LIBRARIES}
        PRIVATE ${CURL_LIBRARIES}
        PRIVATE ${Iconv_LIBRARY}
)
