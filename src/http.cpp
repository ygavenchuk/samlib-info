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
#include <iostream>
#include <iconv.h>
#include <curl/curl.h>
#include "http.h"

using namespace http;


std::string toUtf8(const std::string& str)
{
    const auto fromCode = "WINDOWS-1251";
    const auto toCode = "UTF-8";
    const auto ERROR_ICONV_OPEN = (iconv_t) - 1;
    const auto ERROR_ICONV_CONVERT = (size_t) - 1;

    iconv_t convertingDescriptor = iconv_open(toCode, fromCode);
    if (convertingDescriptor == ERROR_ICONV_OPEN) {
        throw "iconv open failed";  // fixme: add a custom error class
    }

    size_t inSize = str.size();
    char* inBuf = new char[inSize + 1]; strcpy(inBuf, str.c_str());
    char* origInBuf = inBuf; // Save original inBuf pointer to delete it later

    size_t outSize = inSize * 4;
    char* outBuf = new char[outSize];
    char* origOutBuf = outBuf; // Save original outBuf pointer to delete it later

    if (iconv(convertingDescriptor, &inBuf, &inSize, &outBuf, &outSize) == ERROR_ICONV_CONVERT) {
        throw "iconv failed";  // fixme: add a custom error class
    }
    *outBuf = 0; // Null-terminate the output string

    std::string result(origOutBuf);

    delete[] origInBuf;
    delete[] origOutBuf;

    iconv_close(convertingDescriptor);

    return result;
}


// The libcurl callback function that is called after each chunk of data is received
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    // probably it might be convenient to change encoding here
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// The fetchHtml function that accepts a URL as input and uses libcurl to make a GET request to that URL, returning the HTML contents as a string
Page http::get(const std::string &url) {
    CURL* curl = curl_easy_init();
    CURLcode res;
    Page readBuffer;

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            // fixme: throw an exception here
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        long httpCode = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &httpCode);
        if (httpCode != 200 ) { // todo: add explicit status "not found"
            readBuffer = "";
        }

        curl_easy_cleanup(curl);

    }

    return toUtf8(readBuffer);
}
