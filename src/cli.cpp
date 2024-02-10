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
#include <iomanip>      // for std::setw, put_time
#include <string>
#include <algorithm>    // max_element
#include <cmath>        // log10
#include <ctime>
#include <boost/program_options.hpp>
#include <boost/locale.hpp>
#include "core/agent.h"
#include "core/db.h"

namespace po = boost::program_options;

struct BookLocal : public db::BookData {
    std::string path{};
    BookLocal() = default;
    BookLocal(const db::BookData& book) {
        this->id = book.id;
        this->link = book.link;
        this->author = book.author;
        this->title = book.title;
        this->form = book.form;
        this->size = book.size;
        this->group_id = book.group_id;
        this->date = book.date;
        this->description = book.description;
        this->author_id = book.author_id;
        this->mtime = book.mtime;
        this->is_new = book.is_new;
        this->opts = book.opts;
        this->delta_size = book.delta_size;
    }
};


/**
 * @brief Calculates the space width required to print the maximum ID in a sequence.
 *
 * This function determines the number of digits in the maximum ID value in the given sequence,
 * in order to properly align the data when printing. The sequence should contain objects of type
 * db::DBData, which must have an 'id' member variable of type int.
 *
 * @param sequence The sequence of db::DBData objects.
 *
 * @return The space width required to print the maximum ID in the sequence.
 */
unsigned int _getSpaceWidth(const auto& sequence) {
    if (sequence.empty()) {
        return 1;
    }

    unsigned int maxId = std::max(
        std::max_element(
            sequence.begin(), sequence.end(), [](const auto& a, const auto& b){return a.id < b.id;}
        )->id,
        10
    );

    return static_cast<unsigned int>(std::ceil(std::log10(maxId)));
}

/**
 * @brief Formats a description by splitting it into lines based on a maximum width.
 *
 * This function takes a UTF-8 encoded description and splits it into lines based on a maximum width by a space
 * character. When a space character is encountered and the current line length exceeds the maximum width, the line is
 * added to the output list. At the end, any remaining characters are added as a final line.
 * The formatted lines are returned as a vector of strings.
 *
 * @param description The UTF-8 encoded description to format.
 * @param maxWidth The maximum width for each line.
 *
 * @return A vector of UTF-8 encoded strings representing the formatted lines.
 */
std::vector<std::string> formatDescription(const std::string& description, unsigned int maxWidth) {
    // Convert UTF-8 string to wstring
    std::wstring ws = boost::locale::conv::utf_to_utf<wchar_t>(description);

    std::vector<std::string> formattedStrings;
    std::wstring buffer;

    for(auto c : ws) {
        buffer.push_back(c);
        if (c == L' ' && buffer.length() > maxWidth) {
            formattedStrings.push_back(boost::locale::conv::utf_to_utf<char>(buffer));
            buffer.clear();
        }
    }

    // At the end, add the remaining characters
    if (!buffer.empty()) {
        formattedStrings.push_back(boost::locale::conv::utf_to_utf<char>(buffer));
    }

    return formattedStrings;
}


std::ostream& operator<<(std::ostream& out, const db::Authors& authors) {
    const auto width = _getSpaceWidth(authors);
    for (const auto &author: authors) {
        out << "[" << std::setw(width) << std::setfill(' ') << author.id << "]"
            << (author.is_new ? "*" : " ") << " \"" << author.name << "\"" << std::endl;
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const db::GroupBooks& groups) {
    const auto width = _getSpaceWidth(groups);
    for (const auto& group : groups) {
        out << "[" << std::setw(width) << std::setfill(' ') << group.id << "]"
            << (group.new_number ? "*" : " ") << " \"" << group.name << "\"" << std::endl;
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const db::Books& books) {
    const auto width = _getSpaceWidth(books);
    for (const auto& book : books) {
        out << "[" << std::setw(width) << std::setfill(' ') << book.id << "]"
            << (book.is_new ? "*" : " ") << " \"" << book.title << "\"" << std::endl;
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const db::AuthorData& author) {
    auto mtime = author.mtime / 1000;   // author.mtime is stored as number of milliseconds
    out << std::endl;
    out << "          ID: | " << author.id << std::endl;
    out << "        Name: | " << author.name << std::endl;
    out << "         URL: | " << http::toUrl(author.url) << std::endl;
    out << " Has updates: | " << (author.is_new ? "Yes" : "No") << std::endl;
    out << "  Checked at: | " << std::put_time(std::localtime(&mtime), "%Y-%m-%d %H:%M:%S") << std::endl;
    out << std::endl;
    return out;
}

std::ostream& operator<<(std::ostream& out, const db::GroupBookData& group) {
    out << std::endl;
    out << "            ID: | " << group.id << std::endl;
    out << "     Author ID: | " << group.author_id << std::endl;
    out << "         Title: | " << group.display_name << std::endl;
    out << " Total updates: | " << group.new_number << std::endl;
    out << "     Is Hidden: | " << (group.is_hidden ? "Yes" : "No" ) << std::endl;
    out << std::endl;
    return out;
}

std::ostream& operator<<(std::ostream& out, const BookLocal& book) {
    auto updateTime = book.mtime / 1000;   // book.mtime is stored as number of milliseconds
    auto createTime = book.date / 1000;   // book.mtime is stored as number of milliseconds
    unsigned char maxLineWidth = 79 - 18; // here `18` is length of prefix
    auto descriptionChunks = formatDescription(book.description, maxLineWidth);

    out << std::endl;
    out << "            ID: | " << book.id << std::endl;
    out << "     Author ID: | " << book.author_id << std::endl;
    out << "        Author: | " << book.author << std::endl;
    out << "         Title: | " << book.title << std::endl;
    out << "         Genre: | " << book.form << std::endl;
    out << "   Description: | " << (descriptionChunks.empty() ? "" : descriptionChunks[0]) << std::endl;

    if (descriptionChunks.size() > 1) {
        for (auto chunk = std::next(descriptionChunks.begin()); chunk != descriptionChunks.end(); ++chunk) {
            out << "                | " << *chunk << std::endl;
        }
    }

    out << "           URL: | " << http::toUrl(book.link) << ".shtml" << std::endl;
    out << "          Path: | " << "file://" << book.path <<  std::endl;
    out << "  Size (delta): | " << book.size << " (" << book.delta_size << ")" << std::endl;
    out << "   Has updates: | " << (book.is_new ? "Yes" : "No") << std::endl;
    out << "    Created at: | " << std::put_time(std::localtime(&createTime), "%Y-%m-%d %H:%M:%S") << std::endl;
    out << "    Checked at: | " << std::put_time(std::localtime(&updateTime), "%Y-%m-%d %H:%M:%S") << std::endl;
    out << std::endl;
    return out;
}



struct isValidListTarget {
    void operator()(const std::string& v) const {
        if(v != "authors" && v != "a" && v != "groups" && v != "g" && v != "books" && v != "b") {
            throw po::validation_error(po::validation_error::invalid_option_value);
        }
    }
};

struct isValidMarkAction {
    void operator()(const std::string& v) const {
        if(v != "read" && v != "r" && v != "unread" && v != "u") {
            throw po::validation_error(po::validation_error::invalid_option_value);
        }
    }
};


namespace boost { // ensure ID are non-negative
    void validate(boost::any &v, const std::vector<std::string> &values, unsigned int *, int) {
        // Make sure no previous assignment to 'v' was made
        po::validators::check_first_occurrence(v);

        // Extract the string from 'values'. If there is more than one string, it's an error.
        const std::string &s = po::validators::get_single_string(values);

        // Try to parse unsigned int from string s and assign it to v
        try {
            int temp = std::stoi(s);
            if (temp < 0) {
                throw std::invalid_argument("ID must be non-negative.");
            }
            v = static_cast<unsigned int>(temp);
        }
        catch (const std::invalid_argument &) {
            throw po::validation_error(po::validation_error::invalid_option_value, s, "value");
        }
    }
}

void handleList(const po::variables_map& vm, const std::unique_ptr<agent::Agent>& agent) {
    const auto target = vm["list"].as<std::string>();
    if (target.starts_with("a")) {
        std::cout << agent->getAuthors(vm.count("new-only"));
    }
    else if (target.starts_with("g")) {
        if (!vm.count("author")) {
            throw po::error("Please set up authorId (e.g. `--author=123`)");
        }
        auto authorId = vm["author"].as<unsigned int>();
        try{
            std::cout << agent->getGroups(authorId, vm.count("new-only"));
        }
        catch  (db::DoesNotExist& err) {
            std::cerr << "The author #" << authorId << " does not exists in the DB." << std::endl;
        }
    }
    else {
        if (!vm.count("author")) {
            throw po::error("Please set up authorId (e.g. `--author=123`)");
        }

        const auto authorId = vm["author"].as<unsigned int>();
        try {
            std::cout << agent->getBooks<db::Author>(authorId, vm.count("new-only"));
        }
        catch  (db::DoesNotExist& err) {
            std::cerr << "The author #" << authorId << " does not exists in the DB." << std::endl;
        }
    }
}


void handleMarkAs(const po::variables_map& vm, const std::unique_ptr<agent::Agent>& agent) {
    const auto action = vm["mark-as"].as<std::string>();
    if (action.starts_with("r")) {
        if (vm.count("author")) {
            auto authorId = vm["author"].as<unsigned int>();
            try {
                agent->markAsRead<db::Author>(authorId);
            }
            catch  (db::DoesNotExist& err) {
                std::cerr << "The author #" << authorId << " does not exists in the DB." << std::endl;
            }
        }
        else if (vm.count("group")) {
            auto groupId = vm["group"].as<unsigned int>();
            try {
                agent->markAsRead<db::GroupBook>(groupId);
            }
            catch  (db::DoesNotExist& err) {
                std::cerr << "The group #" << groupId << " does not exists in the DB." << std::endl;
            }
        }
        else {
            auto bookId = vm["book"].as<unsigned int>();
            try {
                agent->markAsRead<db::Book>(bookId);
            }
            catch  (db::DoesNotExist& err) {
                std::cerr << "The book #" << bookId << " does not exists in the DB." << std::endl;
            }
        }
    }
    else {
        if (vm.count("author")) {
            throw po::error("Marking author as unread is not supported");
        }
        else if (vm.count("book")) {
            auto bookId = vm["book"].as<unsigned int>();
            try {
                agent->markAsUnRead<db::Book>(bookId);
            }
            catch  (db::DoesNotExist& err) {
                std::cerr << "The book #" << bookId << " does not exists in the DB." << std::endl;
            }
        }
        else {
            throw po::error("Marking book groups as unread is not supported");
        }
    }
}

void handleShow(const po::variables_map& vm, const std::unique_ptr<agent::Agent>& agent) {
    if (vm.count("author")) {
        const auto authorId = vm["author"].as<unsigned int>();
        try {
            std::cout << agent->getAuthor(authorId);
        }
        catch (db::DoesNotExist& err) {
            std::cerr << "The author #" << authorId << " does not exists in the DB." << std::endl;
        }
    }
    else if (vm.count("group")) {
        const auto groupId = vm["group"].as<unsigned int>();
        try {
            std::cout << agent->getGroup(groupId);
        }
        catch  (db::DoesNotExist& err) {
            std::cerr << "The group #" << groupId << " does not exists in the DB." << std::endl;
        }
    }
    else if (vm.count("book")) {
        const auto bookId = vm["book"].as<unsigned int>();

        try {
            auto book = BookLocal(agent->getBook(bookId));

            book.path = agent->getPathToBook(book);

            if (vm.count("path-only")) {
                std::cout << book.path << std::endl;
            }
            else {
                std::cout << book;
            }
        }
        catch  (db::DoesNotExist& err) {
            std::cerr << "The book #" << bookId << " does not exists in the DB." << std::endl;
        }
    }
}

int main(int argc, char **argv) {
    // Define and parse the program options
    po::options_description desc("I know how to");
    desc.add_options()
            ("help", "Show this help messages")
            ("check-updates,u", "Check for updates on all registered authors")
            ("add", po::value<std::string>(), "Add new author")
            ("remove", po::value<unsigned int>(), "Remove author with given ID")
            (
                    "list,l",
                    po::value<std::string>()->notifier(isValidListTarget()),
                    "List [a[uthors]|g[roups]|b[ooks]]. For books or groups you have to specify the `--author` option"
            )
            (
                    "mark-as,m",
                    po::value<std::string>()->notifier(isValidMarkAction()),
                    "Mark as [r[ead]|u[nread]] -a|--author|-b|--book|-g|--group ID"
            )
            ("show,s", "Show -a|--author|-b|--book|-g|--group ID")
            ("author,a", po::value<unsigned int>(), "AuthorID")
            ("book,b", po::value<unsigned int>(), "BookID")
            ("group,g", po::value<unsigned int>(), "GroupID")
            ("new-only,n", "List only new/updated items")
            ("path-only", "Show only path to the local copy of the book with given BookID")
            (
                "location",
                po::value<std::filesystem::path>()->default_value("~/.local/share/SamLib/"),
                "Path to application data (e.g. DB, book storage etc)"
            )
    ;

    po::variables_map vm;

    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm); // can throw

        if (argc == 1 || vm.count("help") || (argc == 2 && vm.count("location") && !vm["location"].defaulted()))
        {
            std::cout << "Please say what should I do:" << std::endl << std::endl << desc << std::endl;
            return 0;
        }

        const auto path = vm["location"].as<std::filesystem::path>();
        auto logger = std::make_shared<logger::Logger>();
        logger->setLogLevel(logger::LogLevel::Info);
        auto agent = std::make_unique<agent::Agent>(path / "samlib.db", path, logger);
        agent->initDB();

        if (vm.count("check-updates")) {
            agent->checkUpdates();
        }
        else if (vm.count("add")) {
            agent->addAuthor(vm["add"].as<std::string>());
        }
        else if (vm.count("remove")) {
            // Note: this action doesn't affect downloaded books! All books of the author that were downloaded
            //       earlier are left untouched.
            agent->removeAuthor(vm["remove"].as<unsigned int>());
        }
        else if (vm.count("list")) {
            handleList(vm, agent);
        }
        else if (vm.count("mark-as")) {
            handleMarkAs(vm, agent);
        }
        else if (vm.count("show")) {
            handleShow(vm, agent);
        }

        // call notify function on each option container to run the assigned tasks
        // it also checks option dependencies and can throw exceptions
        po::notify(vm);
    }
    catch(po::error& e)
    {
        std::cerr << "Error: " << e.what() << std::endl <<std::endl;
        std::cerr << desc << std::endl;
        return -1;
    }

    return 0;
}
