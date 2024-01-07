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


#ifndef SAMLIBINFO_PARSER_H
#define SAMLIBINFO_PARSER_H

#include <vector>
#include <string>

namespace parser {
    // const auto DEFAULT_BOOK_PATTERN = R"lit(^<DL><DT><li>(?:<font.*?<\/font>)?<A\s+HREF=([^<>]+)\.html><b>(.*?)<\/b><\/A>\s+&nbsp;\s+<b>(\d+)k<\/b>\s+&nbsp;\s+<small>(?:.*?<\/b>\s+&nbsp;)?\s+([^<>]+)?\s+(?:<A\s+HREF="\/comment.*?<DD>)?(?:<font\s+color="#555555">([^<>]+)<\/font>)?.*<\/DL>$)lit";
    const auto DEFAULT_BOOK_PATTERN =
            R"lit(^<DL><DT><li>)lit"
            R"lit((?:(?:<font.*?<\/font>))lit"               // update marker (something like "Upd." prefix)
            R"lit(|(?:\s*<b>.*<\/b>\s*))?)lit"               // co-author marker (other prefix)
            R"lit(<A\s+HREF=([^<>]+)\.shtml><b>)lit"         // url
            R"lit((.*?))lit"                                 // title
            R"lit(<\/b><\/A>\s+&nbsp;\s+<b>)lit"
            R"lit((\d+))lit"                                 // size
            R"lit(k<\/b>\s+&nbsp;\s+<small>)lit"
            R"lit((?:.*?<\/b>\s+&nbsp;)?\s*)lit"             // score
            R"lit(([^<>]+)?)lit"                             // genre
            R"lit(\s*(?:<A\s+HREF="\/comment.*?<DD>)?)lit"   // comment
            R"lit((?:<font\s+color="#555555">)lit"
            R"lit(([^<>]+))lit"                              // description
            R"lit(<\/font>)?)lit"
            R"lit(.*<\/DL>$)lit"
    ;

//    const auto DEFAULT_BOOK_GROUPS_PATTERN = R"lit(name=gr\d+>(<a.*?393939>)?(.*?)(<\/font><\/a>)?<gr\d+>([\S\s]*?)((<\/small><p><font.*?)|(<\/dl>)))lit";
    const auto DEFAULT_BOOK_GROUPS_PATTERN =
            R"lit(<a\s+name=gr\d+>)lit"
            R"lit((?:<a\s+href=([^<>]+)\.shtml><font\s+color=#393939>)?)lit"  // URL of extended group
            R"lit(([^<>]+))lit"                                               // group name
            R"lit((?:<\/font><\/a>)?)lit"                                     // some final tags for group URL
            R"lit((?:<gr\d+>)?)lit"                                           // closing tag after group name
            R"lit(([\S\s]*?))lit"                                             // the main group content - list of books
            R"lit((?:(?:<\/small><p><font.*?))lit"                            // in fact - beginning of the next group
            R"lit(|(?:<\/dl>)))lit"                                           // or end of the main page content
    ;


    const auto DEFAULT_AUTHOR_PATTERN =
            R"lit(^<h3>)lit"              // author's name tag
            R"lit(([^<>]*)<br>)lit"       // author's name,
            R"lit((?:\s+<font[^<>]+>)lit" // in fact, there's a `\n` between author's name and description
            R"lit(([^<>]+))lit"           // description, some extra information about author
            R"lit(<\/font>)?<\/h3>$)lit"  // final tags
    ;

    enum BookGroupType  {BookGroupPlain, BookGroupExternal};

    struct Book {
        unsigned int size;
        std::string url;
        std::string title;
        std::string genre;
        std::string description;

        Book(): size(0) {}
    };
    using BooksList = std::vector<Book>;

    struct BookGroup {
        BookGroupType type;
        std::string url;
        std::string name;
        BooksList books;

        BookGroup(): type(BookGroupPlain) {}
    };

    struct Author {
        std::string name;
        std::string description;
    };

    using BookGroupsList = std::vector<BookGroup>;


    BooksList getBooks(const std::string& pageText, const std::string& bookPattern = DEFAULT_BOOK_PATTERN);
    BookGroupsList getBookGroupList(const std::string& pageText, const std::string& bookGroupPattern = DEFAULT_BOOK_GROUPS_PATTERN);
    Author getAuthor(const std::string& pageText, const std::string& pattern = DEFAULT_AUTHOR_PATTERN);
}

#endif //SAMLIBINFO_PARSER_H
