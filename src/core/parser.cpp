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
#include "core/parser.h"
#include "core/tools.h"

using namespace parser;

// todo: extract genre i.e. SciFi, adventure, detective etc (@see db::AuthorData.form)
BooksList parser::getBooks(const std::string& pageText, const std::string& bookPattern) {
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
        book.genre = match[4].str();
        book.description = match[5].str();

        bookList.push_back(book);
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

        bookGroupsList.push_back(bookGroup);
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
