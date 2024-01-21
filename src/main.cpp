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

#include <iostream>
#include <string>
#include "core/agent.h"


int main(int argc, char **argv) {
    //// Define path to your data locations. E.g. let's use $XDG_DATA_HOME (i.e. `~/.local/share`)
    //// @see https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
     const std::string path = "~/.local/share/SamLib/";

    //// First of all you need to specify path to the SQLite DB file
    //// You may use DB exported from the original SamLibInfo app or create an empty DB
    std::string dbFile(path + "samlib.db");

    //// Now, let's configure a logger
    auto logger = std::make_shared<logger::Logger>();
    logger->setLogLevel(logger::LogLevel::Info);

    //// Create an `Agent` instance to start working with the app
    auto agent = agent::Agent(dbFile, path, logger);
    //// in case of empty DB don't forget to init the DB (create all required tables)
    //// Note, that the `initDB()` method checks if a table exists before attempt to creat it
    //// so, it's safe to keep this line uncommented
    agent.initDB();

    //// If you use an empty DB, or you want to check how this functionality works
    //// - uncomment next line(s)
    // agent.addAuthor("http://samlib.ru/s/sedrik/");
    // agent.addAuthor("/s/saggaro_g/");

    //// try to fetch some book as FB2.
    /// NOTE, in case the site hasn't requested book in the FB2 format it will be automatically saved as HTML
    // agent.fetchBook(19);

    //// or you may explicitly store it as HTML if you like
    // agent.fetchBook(19, fs::BookType::HTML);

    //// If log messages are "slightly" noisy - you may change the default log level by uncommenting
    //// and/or changing the next line `src/agent.cpp:18` (in the `agent::Agent::Agent`)

    //// you may mark any Book as (un)red by specifying its ID (or instance of the db::BookData)
    // agent.markAsUnRead<db::Book>(6825);

    //// you may mark any Group of Books as red by specifying its ID (or instance of the db::GroupBookData)
    //// you cannot "Un-read" group though
    // agent.markAsRead<db::GroupBook>(801);

    //// you may mark any Author as red by specifying its ID (or instance of the db::AuthorData)
    //// you cannot "Un-read" author though
    // agent.markAsRead<db::Author>(1);

    //// you may completely remove author by its ID (or instance of the db::AuthorData)
    // agent.removeAuthor(122);

    //// To see updates for all known authors (authors that had been added to the DB)
    //// uncomment the next 4 lines
    // std::cout << "\n==== Authors ====" << std::endl;
    // for(const auto& author: agent.getAuthors(true)) {
    //     std::cout << "[" << author.id << "]" << " \"" << author.name << "\" (" << agent.countBooks(author, true) << ")" << std::endl;
    //}

    //// To see updates in a specific group
    // std::cout << "\n==== Groups ====" << std::endl;
    // for(const auto& group: agent.getGroups(1, true)) {
    //     std::cout << "[" << group.id << "]" << " \"" << group.name << "\" (" << agent.countBooks(group, true) << ")" << std::endl;
    // }

    //// To see information about all updates books in a specific group
    // std::cout << "\n==== Books ====" << std::endl;
    // for(const auto& book: agent.getBooks<db::GroupBook>(1, true)) {
    //     std::cout << "[" << book.id << "]" << " \"" << book.title << "\""
    //               << " (" << book.size << "k ±" << book.delta_size << "k)" << std::endl;
    // }

    return 0;
}
