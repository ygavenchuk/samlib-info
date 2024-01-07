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

#include "agent.h"

using namespace agent;

void Agent::initDB() {
    this->_tAuthor->createTable();
    this->_tGroup->createTable();
    this->_tBook->createTable();
}

Agent::Agent(const std::string& dbPath) :
    _con(db::Connection(dbPath)), _miner(_con, _logger)
{
    // this->_logger.setLogLevel(logger::LogLevel::Warning);
    this->_tAuthor = new db::DB<db::Author>(this->_con);
    this->_tBook = new db::DB<db::Book>(this->_con);
    this->_tGroup = new db::DB<db::GroupBook>(this->_con);
}


Agent::~Agent() {
    delete this->_tAuthor;
    this->_tAuthor = nullptr;

    delete this->_tBook;
    this->_tBook = nullptr;

    delete this->_tGroup;
    this->_tGroup = nullptr;
}

void Agent::checkUpdates() {
    this->_miner.syncAll();
}

db::Authors Agent::getAuthors(bool updatesOnly) {
    return this->_tAuthor->retrieve(
        updatesOnly? static_cast<db::Where>(db::WhereIsNew<db::Author>()) : db::WhereAny()
    );
}

template<>
db::Books Agent::getBooks<db::Author>(unsigned int id, bool updatesOnly) {
    return this->_tBook->retrieve(
        updatesOnly ? db::WhereIdIs<db::Author>(id) & db::WhereIsNew<db::Book>() : db::WhereIdIs<db::Author>(id)
    );
}

template<>
db::Books Agent::getBooks<db::GroupBook>(unsigned int id, bool updatesOnly) {
    return this->_tBook->retrieve(
        updatesOnly ? db::WhereIdIs<db::GroupBook>(id) & db::WhereIsNew<db::Book>() : db::WhereIdIs<db::GroupBook>(id)
    );
}

db::Books Agent::getBooks(const db::AuthorData &author, bool updatesOnly) {
    return this->_tBook->retrieve(
        updatesOnly ? db::WhereAuthorIs(author) & db::WhereIsNew<db::Book>() : db::WhereAuthorIs(author)
    );
}

db::GroupBooks Agent::getGroups(unsigned int authorId, bool updatesOnly) {
    return this->_tGroup->retrieve(
        updatesOnly
        ? db::WhereIdIs<db::Author>(authorId) & db::WhereIsNew<db::GroupBook>()
        : db::WhereIdIs<db::Author>(authorId)
    );
}

db::GroupBooks Agent::getGroups(const db::AuthorData &author, bool updatesOnly) {
    return this->_tGroup->retrieve(
        updatesOnly ? db::WhereAuthorIs(author) & db::WhereIsNew<db::GroupBook>() : db::WhereAuthorIs(author)
    );
}

template<>
unsigned int Agent::countBooks<db::GroupBook>(unsigned int id, bool updatesOnly) {
    return this->_tBook->count(
        updatesOnly ? db::WhereIdIs<db::GroupBook>(id) & db::WhereIsNew<db::Book>() : db::WhereIdIs<db::GroupBook>(id)
    );
}

template<>
unsigned int Agent::countBooks<db::Author>(unsigned int id, bool updatesOnly) {
    return this->_tBook->count(
        updatesOnly ? db::WhereIdIs<db::Author>(id) & db::WhereIsNew<db::Book>() : db::WhereIdIs<db::Author>(id)
    );
}

unsigned int Agent::countBooks(const db::AuthorData& author, bool updatesOnly) {
    return this->countBooks<db::Author>(author.id, updatesOnly);
}

unsigned int Agent::countBooks(const db::GroupBookData& group, bool updatesOnly) {
    return this->countBooks<db::GroupBook>(group.id, updatesOnly);
}

unsigned int Agent::countGroups(const db::AuthorData &author, bool updatesOnly) {
    return this->_tGroup->count(
        updatesOnly ? db::WhereAuthorIs(author) & db::WhereIsNew<db::Book>() : db::WhereAuthorIs(author)
    );
}

db::AuthorData Agent::addAuthor(const std::string& url) {
    db::AuthorData author;
    try {
        author = this->_miner.getAuthor(url);
    }
    catch (miner::AuthorNotFound& err) {
        return author;
    }
    catch (miner::InvalidURL& err) {
        return author;
    }
    //todo: check if author with this URL already exists in the db
    if (this->_tAuthor->exists(db::Where("URL='" + author.url + "'"))) {
        this->_logger.warning << "Author \"" << author.name << "\" already is in the DB. " << std::endl;
    }
    else {
        author = this->_tAuthor->add(author);
    }
    this->_miner.sync(author);
    return author;
}


// fixme: move real column names somewhere into the "db.h", this class/file shouldn't know anything about it!
template<>
void Agent::markAsRead<db::Author>(unsigned int id) {
    this->_tAuthor->begin();
    this->_tBook->update(db::WhereMe(id), "ISNEW=0", "DELTA_SIZE=0");
    this->_tGroup->update(db::WhereMe(id), "NEW_NUMBER=0");
    this->_tAuthor->update(db::WhereMe(id), "ISNEW=0");
    this->_tAuthor->commit();
}

void Agent::markAsRead(const db::AuthorData& author) {
    this->markAsRead<db::Author>(author.id);
}

template<>
void Agent::markAsRead<db::Book>(unsigned int id) {
    this->_tBook->begin();
    db::BookData book;

    try {
        book = this->_tBook->get(id);
    } catch (db::DoesNotExist& err) {
        this->_logger.error << err.what() << std::endl;
        this->_tBook->rollback();
        return;
    }

    this->_tBook->update(db::WhereMe(book), "ISNEW=0", "DELTA_SIZE=0");

    if (book.group_id > 0) {    // some authors may not have any groups
//        auto totalNewBooks = this->_tGroup->count(db::WhereMe(book.group_id) & db::WhereIsNew<db::GroupBook>());
        auto totalNewBooks = this->countBooks<db::GroupBook>(book.group_id);
        this->_tGroup->update(
            db::WhereMe(book.group_id),
            "NEW_NUMBER = " + std::to_string(totalNewBooks? totalNewBooks - 1 : 0)
        );
    }

//    auto totalNewBooks = this->_tBook->count(db::WhereIsNew<db::Book>() & db::WhereIdIs<db::Author>(book.author_id));
    auto totalNewBooks = this->countBooks<db::Author>(book.author_id);
    this->_tAuthor->update(
        db::WhereMe(book.author_id), "ISNEW=0" + std::to_string(totalNewBooks)
    );

    this->_tBook->commit();
}

void Agent::markAsRead(const db::BookData &book) {
    this->markAsRead<db::Book>(book.id);
}


template<>
void Agent::markAsRead<db::GroupBook>(unsigned int id) {
    this->_tGroup->begin();

    db::GroupBookData group;
    try {
        group = this->_tGroup->get(id);
    } catch (db::DoesNotExist& err) {
        this->_logger.error << err.what() << std::endl;
        this->_tBook->rollback();
        return;
    }

    this->_tBook->update(db::WhereIdIs<db::GroupBook>(id), "ISNEW=0", "DELTA_SIZE=0");
    this->_tGroup->update(db::WhereMe(id), "NEW_NUMBER=0");

//    auto totalNewBooks = this->_tBook->count(db::WhereIsNew<db::Book>() & db::WhereIdIs<db::Author>(group.author_id));
    auto totalNewBooks = this->countBooks<db::Author>(group.author_id);
    this->_tAuthor->update(
        db::WhereMe(group.author_id), "ISNEW=" + std::to_string(totalNewBooks)
    );

    this->_tGroup->commit();
}

void Agent::markAsRead(const db::GroupBookData &group) {
    this->markAsRead<db::GroupBook>(group.id);
}

template<>
void Agent::markAsUnRead<db::Book>(unsigned int id) {
    this->_tBook->begin();
    db::BookData book;

    try {
        book = this->_tBook->get(id);
    } catch (db::DoesNotExist& err) {
        this->_logger.error << err.what() << std::endl;
        this->_tBook->rollback();
        return;
    }

    this->_tBook->update(db::WhereMe(book), "ISNEW=1", "DELTA_SIZE=" + std::to_string(book.size));

    if (book.group_id > 0) {    // some authors may not have any groups
        auto totalNewBooks = this->_tGroup->count(db::WhereMe(book.group_id) & db::WhereIsNew<db::GroupBook>());
        this->_tGroup->update(
            db::WhereMe(book.group_id), "NEW_NUMBER = " + std::to_string(totalNewBooks + 1)
        );
    }

    this->_tAuthor->update(db::WhereMe(book.author_id), "ISNEW=1");

    this->_tBook->commit();
}

void Agent::markAsUnRead(const db::BookData& book) {
    this->markAsUnRead<db::Book>(book.id);
}

void Agent::removeAuthor(unsigned int id) {
    const auto whereAuthorId = db::WhereIdIs<db::Author>(id);
    this->_tAuthor->begin();
    try {
        this->_tBook->remove(whereAuthorId);
        this->_tGroup->remove(whereAuthorId);
        this->_tAuthor->remove(db::WhereMe(id));
    } catch (const db::DBError &err) {
        this->_logger.error << "Cannot remove data for the author #" << id
                            << "due to DB error: \"" << err.what() << "\"" << std::endl;
        this->_tAuthor->rollback();
        return;
    }
    this->_tAuthor->commit();
    this->_logger.debug << "All data about author #" << id << "\" was removed from the DB." << std::endl;
}

void Agent::removeAuthor(const db::AuthorData &author) {
    this->removeAuthor(author.id);
}
