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


#ifndef SAMLIBINFO_AGENT_H
#define SAMLIBINFO_AGENT_H

#include "db.h"
#include "miner.h"
#include "logger.h"

namespace agent {

    class Agent {
        private:
            db::Connection _con;
            logger::Logger _logger;
            miner::Miner _miner;
            db::DB<db::Book>* _tBook;
            db::DB<db::GroupBook>* _tGroup;
            db::DB<db::Author>* _tAuthor;

        public:
            Agent(const std::string& dbPath);
            // Agent(const std::string& dbPath, logger::Logger& logger); // fixme: implement this constructor
            ~Agent();
            void checkUpdates();
            // fixme: refactor this! I'd prefer to have interface like the next one:
            //        me->author->add()
            //        me->author->retrieve()
            //        me->author->count()
            //        me->author->markAsRead()
            //        me->groups->retrieve()
            //        me->...
            void initDB();
            db::AuthorData addAuthor(const std::string& url);
            void removeAuthor(unsigned int id);
            void removeAuthor(const db::AuthorData& author);
            db::Authors getAuthors(bool updatesOnly = false);
            db::GroupBooks getGroups(const db::AuthorData& author, bool updatesOnly = false);
            db::GroupBooks getGroups(unsigned int authorId, bool updatesOnly = false);
            template <typename T>
            db::Books getBooks(unsigned int id, bool updatesOnly = false);
            db::Books getBooks(const db::AuthorData& author, bool updatesOnly = false);
            template <typename T>
            unsigned int countBooks(unsigned int id, bool updatesOnly = false);
            unsigned int countBooks(const db::AuthorData& author, bool updatesOnly = false);
            unsigned int countBooks(const db::GroupBookData& group, bool updatesOnly = false);
            unsigned int countGroups(const db::AuthorData& author, bool updatesOnly = false);
            template <typename T>
            void markAsRead(unsigned int id);
            void markAsRead(const db::AuthorData& author);
            void markAsRead(const db::BookData& book);
            void markAsRead(const db::GroupBookData& group);
            template <typename T>
            void markAsUnRead(unsigned int id);
            void markAsUnRead(const db::BookData& book);
    };
}

#endif //SAMLIBINFO_AGENT_H
