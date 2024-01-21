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

#include "core/db.h"

using namespace db;

std::string Connection::_getCleanPath(const std::string &dbPath) const {
    if (dbPath.empty() || dbPath.starts_with(":memory:") || dbPath.starts_with("file::memory:")) {
        return dbPath;
    }

    try {
        return fs::path::ensure(dbPath);
    }
    catch (fs::FSError &err) {
        throw DBError(static_cast<std::string>("A new DB cannot be created : ") + err.what());
    }
}

Connection::Connection(const std::string& dbPath) {
    int rc = sqlite3_open(this->_getCleanPath(dbPath).c_str(), &this->session);
    if(rc!= SQLITE_OK) {
        throw DBError(static_cast<std::string>("Can't open database: ") + sqlite3_errmsg(this->session));
    }
}

Connection::~Connection() { sqlite3_close(this->session); }


Where::Where(const Where& other) : _value(other._value) {}
Where::operator std::string() const { return this->_value; }
Where::operator bool() const { return !this->_value.empty(); }
bool Where::empty() const { return this->_value.empty(); }

Where Where::operator &(const Where& other) const {
    if (other.empty()) {
        return Where(this->_value);
    }

    if (this->empty()) {
        return Where(static_cast<std::string>(other));
    }

    return Where("(" + this->_value + " AND " + static_cast<std::string>(other) + ")");
}
Where Where::operator |(const Where& other) const {
    if (other.empty()) {
        return Where(this->_value);
    }

    if (this->empty()) {
        return Where(static_cast<std::string>(other));
    }

    return Where("(" + this->_value + " OR " + static_cast<std::string>(other) + ")");
}
Where Where::operator !() const {
    if (this->empty()) {
        return Where("");
    }

    return Where("NOT (" + this->_value + ")");
}

template<> WhereIsNew<db::AuthorData>::WhereIsNew() : Where("ISNEW = 1") {}
template<> WhereIsNew<db::Author>::WhereIsNew() : Where("ISNEW = 1") {}
template<> WhereIsNew<db::BookData>::WhereIsNew() : Where("ISNEW = 1") {}
template<> WhereIsNew<db::Book>::WhereIsNew() : Where("ISNEW = 1") {}
template<> WhereIsNew<db::GroupBookData>::WhereIsNew() : Where("NEW_NUMBER > 0") {}
template<> WhereIsNew<db::GroupBook>::WhereIsNew() : Where("NEW_NUMBER > 0") {}

template<> WhereIdIs<db::AuthorData>::WhereIdIs(unsigned int id) : Where("AUTHOR_ID = " + std::to_string(id)) {}
template<> WhereIdIs<db::Author>::WhereIdIs(unsigned int id) : Where("AUTHOR_ID = " + std::to_string(id)) {}
template<> WhereIdIs<db::GroupBook>::WhereIdIs(unsigned int id) : Where("GROUP_ID = " + std::to_string(id)) {}
template<> WhereIdIs<db::GroupBookData>::WhereIdIs(unsigned int id) : Where("GROUP_ID = " + std::to_string(id)) {}

WhereBookIs::WhereBookIs(const BookData& book) : Where("BOOK_ID = " + std::to_string(book.id)) {}
WhereGroupIs::WhereGroupIs(const GroupBookData& group) : Where("GROUP_ID = " + std::to_string(group.id)) {}
WhereAuthorIs::WhereAuthorIs(const AuthorData& author) : Where("AUTHOR_ID = " + std::to_string(author.id)) {}
