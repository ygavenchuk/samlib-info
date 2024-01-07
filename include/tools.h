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


#ifndef SAMLIBINFO_TOOLS_H
#define SAMLIBINFO_TOOLS_H

#include <iostream>
#include <unordered_set>

// todo: refactor this macros to the normal logging system/class
// Check if the configuration is Debug
#ifdef DEBUG
    // If true, define DEBUG to output its arguments to std::cout
    #define DEBUG_PRINT(...) std::cout << __VA_ARGS__ << std::endl;
    #define dbg std::cout

#else
    // If false (we're not in Debug configuration), define DEBUG_PRINT to do nothing
    #define DEBUG_PRINT(...)
    #define dbg if(false) std::cout
#endif

using Predicate = bool(*)(unsigned char);

inline bool noisyChar(unsigned char ch)
{
    static const std::unordered_set<unsigned char> punct = {',', '.', ':', ';', '@', '-'};
    return !std::isspace(ch) && !punct.contains(ch);
}

inline void ltrim(std::string &s, Predicate until)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), until));
}

inline void ltrim(std::string &s)
{
    ltrim(s, [](unsigned char ch) {return !std::isspace(ch);});
}

inline void rtrim(std::string &s, Predicate until)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), until).base(), s.end());
}

inline void rtrim(std::string &s)
{
    rtrim(s, [](unsigned char ch) {return !std::isspace(ch);});
}

inline void trim(std::string &s, Predicate until)
{
    rtrim(s, until);
    ltrim(s, until);
}

inline std::string trim_copy(std::string s, Predicate until)
{
    rtrim(s, until);
    ltrim(s, until);
    return s;
}

inline void trim(std::string &s)
{
    rtrim(s);
    ltrim(s);
}

unsigned long getLevenshteinDistance(const std::string& text1, const std::string& text2);

#endif //SAMLIBINFO_TOOLS_H
