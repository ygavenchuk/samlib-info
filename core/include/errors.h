/*
 * Copyright 2024 Yurii Havenchuk.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef SAMLIBINFO_ERRORS_H
#define SAMLIBINFO_ERRORS_H

#include <stdexcept>

class SamLibError : public std::runtime_error
{
    public:
        explicit SamLibError(const std::string& arg) : std::runtime_error(arg) {}
        explicit SamLibError(const char* arg) : std::runtime_error(arg) {}
};

class MemoryError : public SamLibError {
    public:
        explicit MemoryError(const std::string& arg) : SamLibError("MemoryError: " + arg) {}
        explicit MemoryError(const char* arg) : SamLibError(std::string("MemoryError: ") + arg) {}
};

#endif //SAMLIBINFO_ERRORS_H
