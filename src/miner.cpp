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

#include <cstdlib>
#include <chrono>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <ranges>
#include <regex>
#include "miner.h"
#include "tools.h"
#include "http.h"

using namespace miner;

using GroupName = std::string;
using Url = std::string;

std::time_t getNow() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

class DbUrlMixin {
    protected:
        const db::AuthorData& _author;

    public:
        explicit DbUrlMixin(const db::AuthorData& author): _author(author) {}

        [[nodiscard]] inline std::string _getDBUrl(const parser::Book &webBook) const {
            return this->_author.url.substr(1) + webBook.url;
        }
};

class StoredBookRegistry: public DbUrlMixin {
    private:
        std::unordered_map<Url, const db::BookData&> _storedBooksMap;
        std::unordered_set<unsigned int> _knownBookIDs;
        const db::AuthorData& _author;

    public:
        StoredBookRegistry(const db::Books& storedBooks, const db::AuthorData& author):
        _author(author), DbUrlMixin(author) {
            for(auto& storedBook : storedBooks) {
                this->_storedBooksMap.emplace(storedBook.link, std::cref(storedBook));
            }
        }

        [[nodiscard]] bool isNew(const parser::Book& webBook) const {
            return this->_storedBooksMap.find(this->_getDBUrl(webBook)) == this->_storedBooksMap.end();
        }

        bool isUpdated(const parser::Book& webBook) {
            const auto& dbBook = this->_storedBooksMap.find(this->_getDBUrl(webBook))->second;
            this->_knownBookIDs.insert(dbBook.id);
            return dbBook.size != webBook.size;
        }

        bool isMoved(const parser::Book& webBook, const db::GroupBookData& maybeNewGroup) {
            const auto& dbBook = this->_storedBooksMap.find(this->_getDBUrl(webBook))->second;
            this->_knownBookIDs.insert(dbBook.id);
            return dbBook.group_id != maybeNewGroup.id;
        }

        const db::BookData& operator[] (const parser::Book& webBook) {
            return this->_storedBooksMap.find(this->_getDBUrl(webBook))->second;
        }

        auto getAbandonedBooks() {
            return this->_storedBooksMap | std::views::values | std::views::filter(
                [this](const db::BookData& storedBook) { return !this->_knownBookIDs.contains(storedBook.id); }
            );
        }
};

class StoredGroupRegistry {
    private:
        std::unordered_map<Url, const db::GroupBookData&> _storedGroupsMap;
        std::unordered_set<unsigned int> _knownGroupIDs;

    public:
        explicit StoredGroupRegistry(const db::GroupBooks &storedGroups) {
            for (const auto &storedGroup: storedGroups) {
                this->_storedGroupsMap.emplace(trim_copy(storedGroup.name, noisyChar), storedGroup);
            }
        }

        const db::GroupBookData& operator[] (const parser::BookGroup& webGroup) {
            return this->_storedGroupsMap.find(webGroup.name)->second;
        }

        [[nodiscard]] bool isNew(const parser::BookGroup &webGroup) {
            //return this->_storedGroupsMap.find(webGroup.name) == this->_storedGroupsMap.end();
            const auto item = this->_storedGroupsMap.find(webGroup.name);

            if (item != this->_storedGroupsMap.end()) {
                this->_knownGroupIDs.insert(item->second.id);
                return false;
            }

            return true;
        }

        // try to implement by using getLevenshteinDistance()
        //bool isUpdated(const parser::BookGroup& webGroup){}

        auto getAbandonedGroups() {
            return this->_storedGroupsMap | std::views::values | std::views::filter(
                [this](const db::GroupBookData& storedGroup) { return !this->_knownGroupIDs.contains(storedGroup.id); }
            );
        }
};

class StoredBookBuilder: public DbUrlMixin {
    private:
        const db::AuthorData& _author;
        StoredBookRegistry& _bookRegistry;
        std::time_t _now;

        db::BookData _web2db(const parser::Book& webBook, db::GroupBookData& group) {
            db::BookData maybeNewBook;
            maybeNewBook.link = this->_getDBUrl(webBook);
            maybeNewBook.author = this->_author.name;
            maybeNewBook.title = webBook.title;
            maybeNewBook.form = webBook.genre;
            maybeNewBook.size = webBook.size;
            maybeNewBook.group_id = group.id;
            maybeNewBook.description = webBook.description;
            maybeNewBook.author_id = this->_author.id;

            return maybeNewBook;
        }


public:
        StoredBookBuilder(const db::AuthorData& author, StoredBookRegistry& bookRegistry) :
            _author(author), _bookRegistry(bookRegistry), DbUrlMixin(author) {
            this->_now = getNow();
        }

        db::BookData buildNew(const parser::Book& webBook, db::GroupBookData& maybeNewGroup) {
            db::BookData newBook = this->_web2db(webBook, maybeNewGroup);
            newBook.date = this->_now;
            newBook.mtime = this->_now;
            newBook.delta_size = webBook.size;
            newBook.is_new = true;

            maybeNewGroup.new_number++; // must be reference!!!

            return newBook;
        }

        db::BookData buildUpdated(const parser::Book& webBook, db::GroupBookData& maybeNewGroup) {
            const auto& storedBook = this->_bookRegistry[webBook];
            unsigned int deltaSize = std::abs((int)(storedBook.size - webBook.size));

            db::BookData updatedBook = this->_web2db(webBook, maybeNewGroup);
            updatedBook.id = storedBook.id;
            updatedBook.date = storedBook.date;
            updatedBook.mtime = this->_now;
            updatedBook.delta_size = deltaSize;
            updatedBook.is_new = true;
            maybeNewGroup.new_number++; // must be reference!!!

            return updatedBook;
        }
};

class StoredGroupBuilder {
    private:
        const db::AuthorData& _author;
        StoredGroupRegistry& _groupRegistry;
        int _groupIndex;

    public:
        StoredGroupBuilder(const db::AuthorData& author, StoredGroupRegistry& registry) :
            _author(author), _groupRegistry(registry), _groupIndex(0) {}

        db::GroupBookData build(const parser::BookGroup& webBookGroup) {
            this->_groupIndex++;

            db::GroupBookData maybeNewGroup;
            maybeNewGroup.name = webBookGroup.name;
            maybeNewGroup.display_name = webBookGroup.name;
            maybeNewGroup.author_id = this->_author.id;

            if (this->_groupRegistry.isNew(webBookGroup)) {
                maybeNewGroup.id = -this->_groupIndex; // assuming in the BookGroup table has no negative IDs
            } else {
                maybeNewGroup.id = this->_groupRegistry[webBookGroup].id;
            }

            return maybeNewGroup;
        }
};

Miner::Miner(const std::shared_ptr<db::Connection>& connection, const std::shared_ptr<logger::Logger>& logger) :
    _logger(logger),
    _con(connection),
    _tAuthor(std::make_shared<db::DB<db::Author>>(_con)),
    _tBook(std::make_shared<db::DB<db::Book>>(_con)),
    _tGroup(std::make_shared<db::DB<db::GroupBook>>(_con))
{}

Miner::Miner(const std::shared_ptr<db::Connection>& connection,
             const std::shared_ptr<logger::Logger>& logger,
             const std::shared_ptr<db::DB<db::Author>>& authorDB,
             const std::shared_ptr<db::DB<db::GroupBook>>& groupDB,
             const std::shared_ptr<db::DB<db::Book>>& bookDB
) :
    _logger(logger), _con(connection), _tAuthor(authorDB), _tBook(bookDB), _tGroup(groupDB)
{}


void Miner::_logDiff(const Difference& diff, const db::AuthorData& author) {
    if (diff.empty()) {
        this->_logger->info << "The page of the author \"" << author.name << "\" has no changes." << std::endl;
    }
    else {
        this->_logger->info << "The changes detected on the page of author \"" << author.name << "\":";
        bool somethingIsSaid = false;
        if (!diff.added.empty()) {
            if (!diff.added.books.empty()) {
                this->_logger->info << " " << diff.added.books.size() << " new book(s)";
                somethingIsSaid = true;
            }

            if (!diff.added.groups.empty()) {
                if (somethingIsSaid) { this->_logger->info << ","; }
                this->_logger->info << " " << diff.added.groups.size() << " new group(s)";
                somethingIsSaid = true;
            }
        }

        if (!diff.updated.empty()) {
            if (!diff.updated.books.empty()) {
                if (somethingIsSaid) { this->_logger->info << ","; }
                this->_logger->info << " " << diff.updated.books.size() << " book(s) updated";
                somethingIsSaid = true;
            }

            if (!diff.updated.groups.empty()) {
                if (somethingIsSaid) { this->_logger->info << ","; }
                this->_logger->info << " " << diff.updated.groups.size() << " group(s) updated";
                somethingIsSaid = true;
            }
        }

        if (!diff.removed.empty()) {
            if (!diff.removed.books.empty()) {
                if (somethingIsSaid) { this->_logger->info << ","; }
                this->_logger->info << " " << diff.removed.books.size() << " book(s) removed";
                somethingIsSaid = true;
            }

            if (!diff.removed.groups.empty()) {
                if (somethingIsSaid) { this->_logger->info << ","; }
                this->_logger->info << " " << diff.removed.groups.size() << " group(s) removed";
            }
        }

        this->_logger->info << "." << std::endl;
    }
}


// todo: try to use getLevenshteinDistance() to identify if groups/books names were slightly modified
Difference miner::Miner::getUpdates(const db::AuthorData& author) {
    this->_logger->info << "Checking updates for the author \"" << author.name << "\"..." << std::endl;
    Difference diff;

    this->_logger->debug << "Fetching data from the author's page \"" << author.url << "\"..."  << std::endl;
    const auto pageText = http::get( http::toUrl(http::S_PROTOCOL, http::S_DOMAIN, author.url));
    if (pageText.empty()) {
        this->_logger->warning << "The page of the author \"" << author.name << "\" (" << author.url
                              << ") cannot be found."  << std::endl;
        diff.isPageRemoved = true;
        return diff;
    }

    const auto criteria =  db::WhereAuthorIs(author);

    const auto storedBooks = this->_tBook->retrieve(criteria);
    this->_logger->debug << "DB contains " << storedBooks.size() << " book(s) of the author \"" << author.name
                        << "\". "  << std::endl;

    const auto storedGroups = this->_tGroup->retrieve(criteria);
    this->_logger->debug << "DB contains " << storedGroups.size() << " book group(s) of the author \"" << author.name
                        << "\". "  << std::endl;

    auto storedBooksRegistry = StoredBookRegistry(storedBooks, author);
    auto storedGroupsRegistry = StoredGroupRegistry(storedGroups);
    auto storedGroupsBuilder = StoredGroupBuilder(author, storedGroupsRegistry);
    auto storedBookBuilder = StoredBookBuilder(author, storedBooksRegistry);

    // todo: handle the case of the mixed structure: some books are in groups and some are not
    auto webBookGroups = parser::getBookGroupList(pageText);
    this->_logger->debug << "parser found " << webBookGroups.size() << " book group(s)."  << std::endl;

    // fixme: refactor this!!!
    // todo: do it concurrenlty (e.g. thread pool)
    for (auto& webBookGroup : webBookGroups) {
        if (!webBookGroup.url.empty()) {
            this->_logger->debug << "Group \"" << webBookGroup.name << "\" is an extended group."
                                << " Fetching data from it (" << author.url << webBookGroup.url << ".shtml) ..."
                                << std::endl;
            const auto groupText = http::get(
                http::toUrl(http::S_PROTOCOL, http::S_DOMAIN, author.url, webBookGroup.url , ".shtml")
            );

            if (groupText.empty()) {
                this->_logger->warning << "Cannot get content of the extended group \"" << webBookGroup.name << "\". "
                                      << "Skipping..."  << std::endl;
            } else {
                const auto extraBooks = parser::getBooks(groupText);
                webBookGroup.books.insert(webBookGroup.books.end(), extraBooks.begin(), extraBooks.end());
            }
        }

        this->_logger->debug << "parser found " << webBookGroup.books.size() << " book(s) in the group \""
                            << webBookGroup.name << "\"." << " Checking..."  << std::endl;

        auto maybeNewGroup = storedGroupsBuilder.build(webBookGroup);

        for (const auto& webBook : webBookGroup.books) {
            if (storedBooksRegistry.isNew(webBook)) {
                this->_logger->debug << "\tBookData \"" << webBook.title << "\" is new. Adding to the result."
                                    << std::endl;
                diff.added.books.push_back(storedBookBuilder.buildNew(webBook, maybeNewGroup));
            } else if (storedBooksRegistry.isUpdated(webBook) || storedBooksRegistry.isMoved(webBook, maybeNewGroup)) {
                auto updatedBook = storedBookBuilder.buildUpdated(webBook, maybeNewGroup);
                if (updatedBook.delta_size != webBook.size) {
                    this->_logger->debug << "\tSize of the \"" << webBook.title << "\" book has been changed. "
                                        << "New size is " << webBook.size << "k"
                                        << " (difference is" << updatedBook.delta_size << "k). "  << std::endl;
                } else {
                    this->_logger->debug << "The \" "<< webBook.title << "\" book was moved to the group"
                                        << " \"" << maybeNewGroup.name << "\"."  << std::endl;
                }

                this->_logger->debug << " Adding to the result."  << std::endl;
                diff.updated.books.push_back(updatedBook);
            } else {
                this->_logger->debug << "\tBookData \"" << webBook.title << "\" is known and it's size remains the same:"
                                    << " " << webBook.size << "k. Skipping..."  << std::endl;
            }
        }

        if (storedGroupsRegistry.isNew(webBookGroup)) {
            this->_logger->debug << "BookData group \"" << webBookGroup.name << "\" is new. Adding to the result.";
            diff.added.groups.push_back(maybeNewGroup);
        } else if (maybeNewGroup.new_number) {
            this->_logger->debug << "BookData group \"" << webBookGroup.name << "\" is changed,"
                                << " it has " << maybeNewGroup.new_number << " new/updated book(s). "
                                << " Adding to the result.";
            diff.updated.groups.push_back(maybeNewGroup);
        } else {
            this->_logger->debug << "BookData group \"" << webBookGroup.name << "\" is known and has no changes.";
        }

        this->_logger->debug << std::endl;
    }

    for (const auto& storedBook : storedBooksRegistry.getAbandonedBooks()) {
        this->_logger->warning << "BookData \"" << storedBook.title << "\""
                              << " was removed by the author. It will be removed from the DB..."  << std::endl;
        diff.removed.books.push_back(storedBook);
    }

    for(const auto& group: storedGroupsRegistry.getAbandonedGroups()) {
        this->_logger->debug << "Group \"" << group.name << "\""
                            << " was removed by the author. It will be removed from the DB..."  << std::endl;
        diff.removed.groups.push_back(group);
    }

    this->_logDiff(diff, author);

    return diff;
}

void Miner::apply(Difference& diff, db::AuthorData& author) {
    if (diff.empty()) {
        this->_logger->debug << "No changes to apply for the author \"" << author.name << "\". Exiting..." << std::endl;
        return;
    }

    // todo: add some flag ("is hidden" or "is removed") instead of removing everyting in this case
    if (diff.isPageRemoved) {
        const auto byAuthor = db::WhereAuthorIs(author);
        this->_tAuthor->begin();
        try {
            this->_tBook->remove(byAuthor);
            this->_tGroup->remove(byAuthor);
            this->_tAuthor->remove(db::WhereMe(author));
        } catch (const db::DBError &err) {
            this->_logger->error << "Cannot remove data for the author \"" << author.name << "\""
                                << "due to DB error: \"" << err.what() << "\"" << std::endl;
            this->_tAuthor->rollback();
            return;
        }
        this->_tAuthor->commit();
        this->_logger->debug << "All data about author \"" << author.name << "\" was removed from the DB." << std::endl;
        return;
    }

    if (!diff.added.empty()) {
        auto groupMap = this->_tGroup->add(diff.added.groups);
        for (auto& book: diff.added.books) {
            book.group_id = groupMap[book.group_id].id;
        }
        this->_tBook->add(diff.added.books);
        this->_logger->debug << "All new group/books of the author \"" << author.name << "\" were added to the DB"
                            << std::endl;
    }

    if (!diff.updated.empty()) {
        this->_tGroup->update(diff.updated.groups);
        this->_tBook->update(diff.updated.books);
        this->_logger->debug << "All updates of group(s)/book(s)s of the author \"" << author.name << "\""
                            << " were saved to the DB" << std::endl;
    }

    if (!diff.removed.empty()) {
        this->_tGroup->remove(diff.removed.groups);
        this->_tBook->remove(diff.removed.books);
        this->_logger->debug << "All group(s)/book(s)s removed by the author \"" << author.name << "\""
                            << " were removed from the DB" << std::endl;
    }

    author.is_new = true;
    author.mtime = getNow();
    this->_tAuthor->update(author);
    this->_logger->debug << "An update marker is added to the author \"" << author.name << "\"" << std::endl;
}

void Miner::sync(db::AuthorData &author) {
    auto diff = this->getUpdates(author);
    this->apply(diff, author);
}

void Miner::syncAll(const std::function<void(const db::AuthorData&, unsigned int, unsigned int)>& progressCallback) {
    unsigned int current = 1;
    auto totalCount = this->_tAuthor->count();
    for(auto& author : this->_tAuthor->retrieve()) {
        this->sync(author);
        progressCallback(author, current, totalCount);
        current++;
    }
}

void Miner::syncAll() {
    this->syncAll([](const db::AuthorData&, unsigned int, unsigned int){});
}

std::string Miner::_getAuthorUrl(const std::string& url) const {
    if (url.empty()) {
        throw miner::InvalidURL("The url \"" + url + "\" isn't a valid author's URL");
    }

    std::regex reAuthorUrl(miner::AUTHOR_URL_PATTERN, std::regex_constants::icase|std::regex_constants::ECMAScript);
    std::smatch matches;
    if (!std::regex_search(url, matches, reAuthorUrl)) {
        throw miner::InvalidURL("The url \"" + url + "\" isn't a valid author's URL");
    }

    if (matches[1].length() > 0) { // check if group 1 is not empty
        return http::toUrl(http::S_PROTOCOL, http::S_DOMAIN, matches[1]);
    }

    std::string result = matches[3];
    return  http::toUrl(http::S_PROTOCOL, http::S_DOMAIN, result.substr(0, 1),  result );
}

inline std::string stripDomain(const std::string& url) {
    auto domainPos = url.find(http::S_DOMAIN);
    if (domainPos > 0) {
        return url.substr(domainPos + http::S_DOMAIN.length(), url.length());
    }

    return url;
}

db::AuthorData Miner::getAuthor(const std::string& url) const {
    const auto canonicalURL = this->_getAuthorUrl(url);
    this->_logger->debug << "Fetching data from the webAuthor's page \"" << canonicalURL << "\"..."  << std::endl;
    const auto pageText = http::get(canonicalURL);
    db::AuthorData dbAuthor;
    if (pageText.empty()) {
        this->_logger->warning << "Cannot find webAuthor's page for the URL \"" << canonicalURL << "\"." << std::endl;
        throw AuthorNotFound("Cannot find webAuthor's page for the URL \"" + canonicalURL + "\".");
    }

    auto webAuthor = parser::getAuthor(pageText);
    dbAuthor.name = webAuthor.name;
    dbAuthor.url = stripDomain(canonicalURL);
    dbAuthor.is_new = true;
    dbAuthor.mtime = getNow();

    return dbAuthor;
}