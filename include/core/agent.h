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
#include "fs.h"

namespace agent {

    class Agent {
        private:
            const std::shared_ptr<db::Connection> _con;
            const std::shared_ptr<logger::Logger> _logger;
            const std::shared_ptr<db::DB<db::Book>> _tBook;
            const std::shared_ptr<db::DB<db::GroupBook>> _tGroup;
            const std::shared_ptr<db::DB<db::Author>> _tAuthor;
            const std::unique_ptr<miner::Miner> _miner;
            const std::unique_ptr<fs::BookStorage> _storage;

            /**
             * @brief Fetches a book from the site by using it's URL from the database and stores it as an HTML file.
             *
             * This function fetches a book from the http://samlib.ru/ and stores it in the book storage as an HTML file
             *
             * @param book The BookData object representing the book to be fetched.
             *
             * @return path to downloaded file if the book was fetched successfully, an empty string otherwise
             */
            std::string _fetchBookAsHTML(const db::BookData& book) const;

            /**
             * @brief Fetches a book from the site by using it's URL from the database in FB2 format.
             *
             * This function fetches a book with the given BookData from the http://samlib.ru/ and stores it in the
             * book storage in FB2 format.
             *
             * @param book The BookData object representing the book to be fetched.
             *
             * @return path to downloaded file if the book was fetched successfully, an empty string otherwise
             */
            std::string _fetchBookAsFB2(const db::BookData& book) const;

        public:
            Agent(const std::string& dbPath, const std::string& bookStorageLocation);
            Agent(const std::string& dbPath, const std::string& bookStorageLocation, const std::shared_ptr<logger::Logger>& logger);
            ~Agent() = default;
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
            db::AuthorData getAuthor(unsigned int authorId);
            db::GroupBooks getGroups(const db::AuthorData& author, bool updatesOnly = false);
            db::GroupBooks getGroups(unsigned int authorId, bool updatesOnly = false);
            db::GroupBookData getGroup(unsigned int groupId);
            template <typename T>
            db::Books getBooks(unsigned int id, bool updatesOnly = false);
            db::Books getBooks(const db::AuthorData& author, bool updatesOnly = false);
            db::BookData getBook(unsigned int bookId);
            std::string getPathToBook(const db::BookData& book);
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

            /**
             * @brief Fetches a book from the http://samlib.ru/.
             *
             * This function fetches a book with the given BookData from the http://samlib.ru/ and stores it in the
             * book storage. The bookType parameter determines the format in which the book will be fetched.
             * The default value is FB2.
             *
             * @param book The BookData object representing the book to be fetched.
             * @param bookType The BookType enum representing the format in which the book will be fetched.
             *                 Default is FB2.
             *
             * @return path to downloaded file if the book was fetched successfully, an empty string otherwise
             */
            std::string fetchBook(const db::BookData& book, fs::BookType bookType = fs::BookType::FB2) const;

            /**
             * @brief Fetches a book from the http://samlib.ru/.
             *
             * This function fetches a book with the given bookId from the http://samlib.ru/ and stores it in the book
             * storage. The bookType parameter determines the format in which the book will be fetched.
             * The default value is FB2.
             *
             * @param bookId The unsigned integer representing the ID of the book to be fetched.
             * @param bookType The BookType enum representing the format in which the book will be fetched.
             *                 Default is FB2.
             *
             * @return path to downloaded file if the book was fetched successfully, an empty string otherwise
             */
            std::string fetchBook(unsigned int bookId, fs::BookType bookType = fs::BookType::FB2) const;
    };
}

#endif //SAMLIBINFO_AGENT_H
