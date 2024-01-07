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

#include <string>
#include "tools.h"

// C++ code for the above approach:

unsigned long levenshteinRecursive(const std::string& text1, const std::string& text2, unsigned long m, unsigned long n)
{
    // text1 is empty
    if (m == 0)
    {
        return n;
    }
    // text2 is empty
    if (n == 0)
    {
        return m;
    }

    if (text1[m - 1] == text2[n - 1])
    {
        return levenshteinRecursive(text1, text2, m - 1, n - 1);
    }

    return 1 + std::min(
            // Insert
            levenshteinRecursive(text1, text2, m, n - 1),
            std::min(
                // Remove
                levenshteinRecursive(text1, text2, m - 1, n),
                // Replace
                levenshteinRecursive(text1, text2, m - 1, n - 1)
            )
    );
}


unsigned long getLevenshteinDistance(const std::string& text1, const std::string& text2)
{
    return levenshteinRecursive(text1, text2, text1.length(), text2.length());
}
