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


#ifndef SAMLIBINFO_HTTP_H
#define SAMLIBINFO_HTTP_H

#include <string>
#include <vector>

namespace http {
    using Page = std::string;

    struct Settings {
        std::string protocol;
        std::string domain;
    };

    /**
     * @brief Get the content from the given URL.
     *
     * This function sends an HTTP GET request to the specified URL and
     * retrieves the content from the response.
     *
     * @note in case response status code is not `200 OK` the function returns an empty string
     *
     * @param url The URL to send the GET request to.
     * @return The content retrieved from the response as a string.
     */
    Page get(const std::string &url);


    /**
     * @brief Convert given paths to a fully qualified URL.
     *
     * This function takes a variable number of paths and combines them with the protocol and domain to create
     * a complete URL.
     * The resulting URL is returned as a string.
     *
     * @param protocol The protocol part of the URL (e.g., "http", "https", etc.).
     * @param domain The domain or hostname part of the URL (e.g., "example.com").
     * @param paths The variable number of paths to be combined with the URL.
     *
     * @return The fully qualified URL as a string.
     *
     * @note This function assumes that the protocol, domain, and paths are provided as string objects.
     *       The paths are treated as individual segments to be combined with the URL.
     *       Empty paths are ignored and not included in the final URL.
     *       The resulting URL is in the format: "protocol://domain/segment1/segment2/..."
     *
     * @example
     *   std::string url1 = toUrl("https", "example.com", "path1", "path2"); // Returns "https://example.com/path1/path2"
     *   std::string url2 = toUrl("ftp", "example.com"); // Returns "ftp://example.com"
     */
    template<typename... Paths>
    std::string toUrl(const std::string& protocol, const std::string& domain, Paths... paths) {
        std::string url = protocol + "://" + domain;
        std::vector<std::string> vec{paths...};
        for(const auto& p : vec) {
            if(!p.empty())
                url += p[0] == '/' || p[0] == '.' ?  p : "/" + p;
        }
        return url;
    }
}

#endif //SAMLIBINFO_HTTP_H
