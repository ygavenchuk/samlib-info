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

#include <regex>
#include "parser.h"
#include "tools.h"

using namespace parser;


/**
 * @class TextCleaner
 *
 * @brief Class to clean text by removing HTML tags, newlines, and multiple spaces.
 *
 * The TextCleaner class provides methods to clean text strings by removing HTML tags, replacing HTML newlines with
 * newlines removing multiple spaces, trimming the leading and trailing spaces, and replacing certain special characters
 */
class TextCleaner {
    private:
        std::regex _reHtmlTags;
        std::regex _reHtmlNewLine;
        std::regex _reMultipleSpaces;

    public:
        explicit TextCleaner() {
            _reHtmlTags.assign("<\\/?(\\S+?)[^>]*?>", std::regex_constants::multiline | std::regex_constants::icase);
            _reHtmlNewLine.assign("<dd>|<br/?>", std::regex_constants::multiline | std::regex_constants::icase);
            _reMultipleSpaces.assign("\\s{2,}", std::regex_constants::multiline);
        }

        /**
         * @brief Cleans the given text by removing HTML tags, newlines, and multiple spaces.
         *
         * This function takes a string `text` as input and performs the following operations to clean the text:
         * 1. Replaces HTML newlines with actual newlines.
         * 2. Removes HTML tags from the text.
         * 3. Replaces multiple spaces with a single space.
         * 4. Trims leading and trailing spaces.
         * 5. Replaces the special character "&#8212;" with a hyphen "-".
         *
         * @param text The text to be cleaned.
         * @return The cleaned version of the input text.
         */
        std::string clean(const std::string& text) {
            std::string cleanText = std::regex_replace(text, _reHtmlNewLine, "\n");
            cleanText = std::regex_replace(cleanText, _reHtmlTags, "");
            cleanText = std::regex_replace(cleanText, _reMultipleSpaces, " ");
            trim(cleanText, [](unsigned char ch){return ch != ' ';});
            replaceAll(cleanText, "&#8212;", "-");

            return cleanText;
        }
};


BooksList parser::getBooks(const std::string& pageText, const std::string& bookPattern) {
    auto textCleaner = std::make_unique<TextCleaner>();

    std::regex reBooks(bookPattern, std::regex_constants::multiline | std::regex_constants::icase);

    BooksList bookList;
    std::sregex_iterator begin = std::sregex_iterator(pageText.begin(), pageText.end(), reBooks);
    std::sregex_iterator end;

    // todo: rewrite by using named groups instead of indices (i.e. `(?<url>...)`, `(?<title>...)` etc
    for (std::sregex_iterator iterator = begin; iterator != end; ++iterator) {
        const std::smatch& match = *iterator;
        Book book;
        book.size = std::stoi(match[3].str());
        book.url = match[1].str();
        book.title = trim_copy(match[2].str(), noisyChar);
        book.genre = trim_copy(match[4].str(), noisyChar);
        book.description = textCleaner->clean(match[5].str());

        bookList.push_back(std::move(book));
    }

    return bookList;
}

BookGroupsList parser::getBookGroupList(const std::string &pageText, const std::string& bookGroupPattern) {
    std::regex reBookGroups(bookGroupPattern, std::regex_constants::ECMAScript);
    BookGroupsList bookGroupsList;
    std::sregex_iterator begin = std::sregex_iterator(pageText.begin(), pageText.end(), reBookGroups);
    std::sregex_iterator end;

    for (std::sregex_iterator i = begin; i != end; ++i) {
        const std::smatch& match = *i;
        const auto url = match[1].str();

        BookGroup bookGroup;
        bookGroup.type = url.empty() ? BookGroupPlain : BookGroupExternal;
        bookGroup.name = trim_copy(match[2].str(), noisyChar);
        bookGroup.books = getBooks(match[3].str());

        // URL that starts from `/type` doesn't belong to the author, it is something common for the whole SamLib site
        // and because it's irrelevant to the author, we don't want to grab that information
        bookGroup.url = url.compare(0, 5, "/type") == 0 ? "" : url ;

        bookGroupsList.push_back(std::move(bookGroup));
    }

    return bookGroupsList;
}


Author parser::getAuthor(const std::string& pageText, const std::string& authorPattern) {
    std::regex reAuthor(authorPattern, std::regex_constants::multiline|std::regex_constants::icase);
    std::sregex_iterator begin = std::sregex_iterator(pageText.begin(), pageText.end(), reAuthor);
    std::sregex_iterator end;

    Author author;

    std::smatch matches;
    if (std::regex_search(pageText, matches, reAuthor)) {
        author.name = trim_copy(matches[1].str(), noisyChar);
        author.description = trim_copy(matches[2].str(), noisyChar);
    }

    return author;
}
