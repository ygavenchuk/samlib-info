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

#ifndef SAMLIBINFO_DB_H
#define SAMLIBINFO_DB_H

#include <utility>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <ctime>
#include <unordered_map>
#include <functional>
#include "sqlite3.h"
#include "errors.h"

namespace db {
    class DBError : public SamLibError {
        public:
            explicit DBError(const std::string& arg) : SamLibError("DBError: " + arg) {}
            explicit DBError(const char* arg) : SamLibError(std::string("DBError: ") + arg) {}
    };

    class QueryError : public DBError {
        public:
            explicit QueryError(const std::string& arg) : DBError("QueryError: " + arg) {}
            explicit QueryError(const char* arg) : DBError(std::string("QueryError: ") + arg) {}
    };

    class DoesNotExist : public DBError {
        public:
            explicit DoesNotExist(const std::string& arg) : DBError("DoesNotExist: " + arg) {}
            explicit DoesNotExist(const char* arg) : DBError(std::string("DoesNotExist: ") + arg) {}
    };

    inline std::string toString(const char* text) {
        return {text == nullptr ? "" : text};
    }

    inline std::string escape(const std::string& text) {
        char *sql = sqlite3_mprintf("%Q", text.c_str());
        std::string statement(sql);
        sqlite3_free(sql);
        return statement;
    }

        /*
    class F {
        private:
            std::pair<std::string, std::string> _name;
            std::string _value;
        public:
            explicit F(std::string name, std::string column) : _name({std::move(name), std::move(column)}) {}
            F& operator =(std::string& value) {
                this->_value = value;
                return *this;
            }
            F& operator =(const char* value) {
                this->_value = std::string(value);
                return *this;
            }
            explicit operator std::string() const { return this->_name.second + "='" + this->_value + "'"; }
    };
    */
    struct DBData {
        int id;
        DBData(): id(0) {};
    };

    struct AuthorData: DBData {
        std::string name;
        std::string url;
        bool is_new;
        time_t mtime;
        std::string all_tags_name;
    };

    struct BookData: DBData {
        std::string link;
        std::string author;
        std::string title;
        std::string form;
        unsigned int size;
        int group_id;
        std::time_t date;
        std::string description;
        int author_id;
        std::time_t mtime;
        bool is_new;
        int opts;
        unsigned int delta_size;

        BookData() : DBData(), size(0), group_id(0), author_id(0), is_new(false),
                     opts(0), delta_size(0), date(0), mtime(0) {}
    };

    struct GroupBookData: DBData {
        int author_id;
        std::string name;
        std::string display_name;
        int new_number;
        bool is_hidden;

        GroupBookData() : DBData(), author_id(0), new_number(0), is_hidden(false) {}
    };

    struct Author {
        AuthorData data;

        static std::string getTable() {return "Author";}
        [[nodiscard]] static std::unordered_map<std::string, std::string> serialize(const AuthorData& author) {
            return std::unordered_map<std::string, std::string> {
                {"NAME", escape(author.name)},
                {"URL", escape(author.url)},
                {"ISNEW", author.is_new ? "1" : "0"},
                {"MTIME", std::to_string(static_cast<long long>(author.mtime))},
                {"ALL_TAGS_NAME", escape(author.all_tags_name)}
            };
        }
        static void load(AuthorData& author, const std::string& fieldName, const char *fieldValue) {
            if (fieldName == "_id") {
                author.id = std::stoi(fieldValue);
            } else if (fieldName == "NAME") {
                author.name = toString(fieldValue);
            } else if (fieldName == "URL") {
                author.url = toString(fieldValue);
            } else if (fieldName == "ISNEW") {
                author.is_new = fieldValue != nullptr && std::stoi(fieldValue);
            } else if (fieldName == "MTIME") {
                // Here you might want to convert the SQL timestamp to time_t format
                author.mtime = static_cast<time_t>(std::stol(fieldValue));
            } else if (fieldName == "ALL_TAGS_NAME") {
                author.all_tags_name = toString(fieldValue);
            }
        }
        static std::string getCrateTableQuery() {
            return std::string(
                "CREATE TABLE IF NOT EXISTS " + Author::getTable() + "(\n"
                "    _id           INTEGER PRIMARY KEY AUTOINCREMENT CHECK (_id >= 0),\n"
                "    NAME          TEXT,\n"
                "    URL           TEXT NOT NULL UNIQUE,\n"
                "    ISNEW         BOOLEAN default '0' NOT NULL,\n"
                "    MTIME         TIESTAMP,\n"
                "    ALL_TAGS_NAME TEXT\n"
                ");\n"
                "\n"
                "CREATE INDEX IF NOT EXISTS idx_author_url ON " + Author::getTable() + " (URL);\n"
                "CREATE INDEX IF NOT EXISTS idx_mtime ON " + Author::getTable() + " (MTIME);"
            );
        }
    };

    struct GroupBook {
        GroupBookData data;
        static std::string getTable() {return "GroupBook";}
        [[nodiscard]] static std::unordered_map<std::string, std::string> serialize(const GroupBookData& group) {
            return std::unordered_map<std::string, std::string> {
                {"AUTHOR_ID", std::to_string(group.author_id)},
                {"NAME", escape(group.name)},
                {"DISPLAY_NAME", escape(group.display_name)},
                {"NEW_NUMBER", std::to_string(group.new_number)},
                {"IS_HIDDEN", group.is_hidden ? "1": "0"}
            };
        }
        static void load(GroupBookData& group, const std::string& fieldName, const char *fieldValue) {
            if(fieldName == "_id") group.id = std::stoi(fieldValue);     // primary key, cannot be null
            else if(fieldName == "AUTHOR_ID") group.author_id = std::stoi(fieldValue);   // not null
            else if(fieldName == "NAME") group.name = toString(fieldValue);
            else if(fieldName == "DISPLAY_NAME") group.display_name = toString(fieldValue);
            else if(fieldName == "NEW_NUMBER") group.new_number = std::stoi(fieldValue); // not null
            else if(fieldName == "IS_HIDDEN") group.is_hidden = fieldValue != nullptr && std::stoi(fieldValue);

        }
        static std::string getCrateTableQuery() {
            return std::string(
                    "CREATE TABLE IF NOT EXISTS " + GroupBook::getTable() + " (\n"
                    "    _id          INTEGER PRIMARY KEY AUTOINCREMENT CHECK (_id >= 0),\n"
                    "    AUTHOR_ID    INTEGER NOT NULL CHECK (AUTHOR_ID >= 0) "
                                      " REFERENCES " + Author::getTable() + "(_id) ON DELETE CASCADE,\n"
                    "    NAME         VARCHAR,\n"
                    "    DISPLAY_NAME VARCHAR,\n"
                    "    NEW_NUMBER   INTEGER NOT NULL CHECK (NEW_NUMBER >= 0),\n"
                    "    IS_HIDDEN    SMALLINT\n"
                    ");\n"
                    "CREATE INDEX IF NOT EXISTS idx_group_author ON " + GroupBook::getTable() + " (NAME, AUTHOR_ID);\n"
            );
        }
    };


    struct Book {
        BookData data;

        static std::string getTable() {return "Book";}
        [[nodiscard]] static std::unordered_map<std::string, std::string> serialize(const BookData& book) {
            return std::unordered_map<std::string, std::string> {
                    {"LINK", escape(book.link)},
                    {"AUTHOR", escape(book.author)},
                    {"TITLE", escape(book.title)},
                    {"FORM", escape(book.form)},
                    {"SIZE", std::to_string(book.size)},
                    {"GROUP_ID", std::to_string(book.group_id)},
                    {"DATE", std::to_string(book.date)},
                    {"DESCRIPTION", escape(book.description)},
                    {"AUTHOR_ID", std::to_string(book.author_id)},
                    {"MTIME", std::to_string(book.mtime)},
                    {"ISNEW", book.is_new ? "1" : "0"},
                    {"OPTS", std::to_string(book.opts)},
                    {"DELTA_SIZE", std::to_string(book.delta_size)}
            };
        }
        static void load(BookData& book, const std::string& fieldName, const char *fieldValue) {
            if(fieldName == "_id"){   // primary key, cannot be null
                book.id = std::stoi(fieldValue);
            }
            else if(fieldName == "LINK"){
                book.link = toString(fieldValue);
            }
            else if(fieldName == "AUTHOR"){
                book.author = toString(fieldValue);
            }
            else if(fieldName == "TITLE"){
                book.title = toString(fieldValue);
            }
            else if(fieldName == "FORM"){
                book.form = toString(fieldValue);
            }
            else if(fieldName == "SIZE"){
                book.size = fieldValue == nullptr ? 0 : std::stoi(fieldValue);
            }
            else if(fieldName == "GROUP_ID"){
                book.group_id = fieldValue == nullptr ? -1 : std::stoi(fieldValue);
            }
            else if(fieldName == "DATE"){
                book.date = static_cast<time_t>(fieldValue == nullptr ? 0 : std::stol(fieldValue));
            }
            else if(fieldName == "DESCRIPTION"){
                book.description = toString(fieldValue);
            }
            else if(fieldName == "AUTHOR_ID"){    // not null
                book.author_id = std::stoi(fieldValue);
            }
            else if(fieldName == "MTIME"){
                book.mtime = static_cast<time_t>(fieldValue == nullptr ? 0 : std::stol(fieldValue));
            }
            else if(fieldName == "ISNEW"){    // not null
                book.is_new = std::stoi(fieldValue);
            }
            else if(fieldName == "OPTS"){
                book.opts = fieldValue == nullptr ? -1 : std::stoi(fieldValue);
            }
            else if(fieldName == "DELTA_SIZE"){
                book.delta_size = fieldValue == nullptr ? 0 : std::stoi(fieldValue);
            }
        }
        static std::string getCrateTableQuery() {
            return std::string(
                    "CREATE TABLE IF NOT EXISTS " + Book::getTable() + "(\n"
                     "    _id         INTEGER PRIMARY KEY AUTOINCREMENT CHECK (_id >= 0),\n"
                     "    LINK        TEXT,\n"
                     "    AUTHOR      TEXT,\n"
                     "    TITLE       TEXT,\n"
                     "    FORM        TEXT,\n"
                     "    SIZE        INTEGER,\n"
                     "    GROUP_ID    INTEGER NOT NULL CHECK (GROUP_ID >= 0) "
                                      " REFERENCES " + GroupBook::getTable() + "(_id) ON DELETE CASCADE,\n"
                     "    DATE        TIMESTAMP,\n"
                     "    DESCRIPTION TEXT,\n"
                     "    AUTHOR_ID   INTEGER NOT NULL CHECK (AUTHOR_ID >= 0) "
                                      " REFERENCES " + Author::getTable() + "(_id) ON DELETE CASCADE,\n"
                     "    MTIME       TIMESTAMP,\n"
                     "    ISNEW       BOOLEAN DEFAULT '0' NOT NULL,\n"
                     "    OPTS        INTEGER,\n"
                     "    DELTA_SIZE  INTEGER\n"
                     ");\n"
                     "CREATE INDEX IF NOT EXISTS idx_book_author ON " + Book::getTable() + " (AUTHOR_ID);\n"
                     "CREATE INDEX IF NOT EXISTS idx_book_mtime ON " + Book::getTable() + " (MTIME);\n"
            );
        }
    };

    using Authors = std::vector<AuthorData>;
    using Books = std::vector<BookData>;
    using GroupBooks = std::vector<GroupBookData>;

    class Where {
        private:
            const std::string _value;

        public:
            explicit Where(std::string where) : _value(std::move(where)) {}
            Where(const Where& other);  // Copy constructor
            explicit operator std::string() const;
            explicit operator bool() const;
            [[nodiscard]] bool empty() const;
            Where operator &(const Where& other) const;
            Where operator |(const Where& other) const;
            Where operator !() const;
    };
    class WhereAny : public Where {
        public:
            explicit WhereAny() : Where("") {}
    };
    template <typename T>
    class WhereIsNew : public Where {
        public:
            WhereIsNew();
    };
    class WhereBookIs: public Where {
        public:
            explicit WhereBookIs(const BookData& book) ;
    };
    class WhereGroupIs: public Where {
        public:
            explicit WhereGroupIs(const GroupBookData& book);
    };
    class WhereAuthorIs: public Where {
        public:
            explicit WhereAuthorIs(const AuthorData& author);
    };

    template <typename T>
    class WhereIdIs: public Where {
        public:
            WhereIdIs(unsigned int id);
    };

    class WhereMe: public Where {
        public:
            explicit WhereMe(const DBData& me) : Where("_id = " + std::to_string(me.id)) {}
            explicit WhereMe(unsigned int id) : Where("_id = " + std::to_string(id)) {}
    };


    class Connection {
        public:
            sqlite3 *session;
            explicit Connection(const std::string &dbPath) {
                int rc = sqlite3_open(dbPath.c_str(), &this->session);
                if(rc!= SQLITE_OK) {
                    std::cerr << "Can't open database: " << sqlite3_errmsg(this->session) << std::endl;
                    throw DBError("Can't open database");
                }
            }

            ~Connection() { sqlite3_close(this->session); }
    };

    template <typename T>
    class DB {
        private:
            const std::shared_ptr<db::Connection> _con;

            int _getCallback(void *data, int numFields, char **fieldValues, char **fieldNames) {
                auto* DBData = static_cast<std::vector<typeof T::data>*>(data);
                typeof T::data dbData;

                for(int i = 0; i < numFields; i++) {
                    std::string columnName = fieldNames[i];
                    T::load(dbData, columnName, fieldValues[i]);
                }

                DBData->push_back(dbData);
                return 0;
            }

            static int staticCallback(void* data, int argc, char** argv, char**azColName) {
                auto parameters = static_cast<std::pair<DB*, std::vector<typeof T::data>*>*>(data);
                return parameters->first->_getCallback(parameters->second, argc, argv, azColName);
            }

            [[nodiscard]] std::string _getPaginator(unsigned int limit, int offset) const {
                std::string paginator;
                if(limit > 0) {
                    paginator.append(" LIMIT " + std::to_string(limit));
                }

                if(offset >= 0) {
                    paginator.append(" OFFSET " + std::to_string(offset));
                }

                return paginator;
            }

        public:
            explicit DB(const std::shared_ptr<db::Connection>& connection) : _con(connection) {}

            void begin() {
                char *zErrMsg = nullptr;
                if (sqlite3_exec(this->_con->session, "BEGIN TRANSACTION;", NULL, NULL, &zErrMsg) != SQLITE_OK) {
                    const std::string errMsg(zErrMsg);
                    sqlite3_free(zErrMsg);
                    throw QueryError(errMsg);
                }
                sqlite3_free(zErrMsg);
            }
            void rollback() {
                sqlite3_exec(this->_con->session, "ROLLBACK;", NULL, NULL, nullptr);
            }
            void commit() {
                char *zErrMsg = nullptr;
                if (sqlite3_exec(this->_con->session, "COMMIT;", NULL, NULL, &zErrMsg) != SQLITE_OK) {
                    const std::string errMsg(zErrMsg);
                    sqlite3_free(zErrMsg);
                    throw QueryError(errMsg);
                }

                sqlite3_free(zErrMsg);
            }

            bool isTableExists() {
                bool exists = false;
                std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + T::getTable() + "'";
                sqlite3_stmt *stmt;

                if (sqlite3_prepare_v2(this->_con->session, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
                    if (sqlite3_step(stmt) == SQLITE_ROW) {
                        exists = true;
                    }
                    sqlite3_finalize(stmt);
                }
                return exists;
            }

            void createTable(){
                char* errMsg;
                if (sqlite3_exec(this->_con->session, T::getCrateTableQuery().c_str(), NULL, 0, &errMsg) != SQLITE_OK) {
                    std::cerr << "Cannot create table: " << errMsg << std::endl;
                    sqlite3_free(errMsg);
                }
            }

            std::vector<typeof T::data> retrieve(const Where& where=WhereAny(), unsigned int limit=0, int offset=-1) {
                std::vector<typeof T::data> dbData;

                // Prepare SQL select statement
                std::string sql = "SELECT * FROM " + T::getTable();
                if (!where.empty()) {
                    sql.append(" WHERE " + static_cast<std::string>(where));
                }

                sql.append(this->_getPaginator(limit, offset) + ";");

                char *zErrMsg = nullptr;
                std::pair<DB*, std::vector<typeof T::data>*> parameters = {this, &dbData};

                int rc = sqlite3_exec(this->_con->session, sql.c_str(), DB::staticCallback, &parameters, &zErrMsg);

                if (rc != SQLITE_OK) {
                    std::string errMsg(zErrMsg);
                    sqlite3_free(zErrMsg);
                    throw QueryError(errMsg.c_str());
                }

                return dbData;
            }
            typeof T::data get(const typeof T::data& dbData) {
                auto result = this->retrieve(WhereMe(dbData), 1);
                if (result.empty()) {
                    throw DoesNotExist(
                        "Cannot find record #" + std::to_string(dbData.id) + " in table " + T::getTable()
                    );
                }
                return result[0];
            }
            typeof T::data get(unsigned int id) {
                auto result = this->retrieve(WhereMe(id), 1);
                if (result.empty()) {
                    throw DoesNotExist(
                        "Cannot find record #" + std::to_string(id) + " in the table \"" + T::getTable() + "\""
                    );
                }
                return result[0];
            }
            typeof T::data get(const Where& where) {
                auto result = this->retrieve(where, 1);
                if (result.empty()) {
                    throw DoesNotExist(
                        "Cannot find record for given criteria in the table \"" + T::getTable() + "\""
                    );
                }
                return result[0];
            }

            void remove(const typeof T::data& dbData) {
                std::string sql = "DELETE FROM " + T::getTable() + " WHERE _id = " + std::to_string(dbData.id) + " LIMIT 1;";

                char *zErrMsg = nullptr;
                int rc = sqlite3_exec(this->_con->session, sql.c_str(), NULL, NULL, &zErrMsg);

                if(rc != SQLITE_OK)
                {
                    std::string errMsg(zErrMsg);
                    sqlite3_free(zErrMsg);
                    throw QueryError(errMsg.c_str());
                }
            }
            void remove(const std::vector<typeof T::data>& dbData) {
                if (dbData.empty()) {
                    return;
                }

                // Generate combined IDs for DELETE query
                std::string ids;
                for (const auto& item : dbData) {
                    ids += std::to_string(item.id) + ",";
                }

                // Remove trailing comma
                ids.pop_back();

                std::string sql = "DELETE FROM " + T::getTable() + " WHERE _id IN (" + ids + ") "
                                  + " LIMIT " + std::to_string(dbData.size());

                char *zErrMsg = nullptr;
                int rc = sqlite3_exec(this->_con->session, sql.c_str(), NULL, NULL, &zErrMsg);

                if( rc != SQLITE_OK ) {
                    std::string errMsg(zErrMsg);
                    sqlite3_free(zErrMsg);
                    throw QueryError(errMsg.c_str());
                }
            }
            void remove(const Where& where) {
                if (where.empty()) {
                    return;
                }

                std::string sql = "DELETE FROM " + T::getTable() + " WHERE " + static_cast<std::string>(where);

                char *zErrMsg = nullptr;
                int rc = sqlite3_exec(this->_con->session, sql.c_str(), NULL, NULL, &zErrMsg);

                if( rc != SQLITE_OK ) {
                    std::string errMsg(zErrMsg);
                    sqlite3_free(zErrMsg);
                    throw QueryError(errMsg.c_str());
                }
            }

            typeof T::data add(const typeof T::data& dbData) {
                std::string sql = "INSERT INTO " + T::getTable();
                std::string columns, values;
                for (const auto& [key, value] : T::serialize(dbData)) {
                    columns += key + ",";
                    values += value + ",";
                }
                columns.pop_back();
                values.pop_back();
                sql.append(" (" + columns + ") ").append("VALUES (" + values + ");");

                char *zErrMsg = nullptr;
                int rc = sqlite3_exec(this->_con->session, sql.c_str(), NULL, NULL, &zErrMsg);

                if(rc != SQLITE_OK) {
                    const std::string errMsg(zErrMsg);
                    sqlite3_free(zErrMsg);
                    throw QueryError(errMsg);
                }

                // Fetch the ID of the last inserted row
                int id = sqlite3_last_insert_rowid(this->_con->session);
                typeof T::data newData = dbData;
                newData.id = id;
                return newData;
            }
            std::unordered_map<int, typeof T::data> add(const std::vector<typeof T::data>& dbDataList) {
                std::unordered_map<int, typeof T::data> newDbDataMap;
                if (dbDataList.empty()) {
                    return newDbDataMap;
                }

                this->begin(); // Begin transaction

                // todo: rewrite SQL query for batch inserting
                char *zErrMsg = nullptr;
                for (const auto &dbDataItem: dbDataList) {
                    try {
                        newDbDataMap[dbDataItem.id] = this->add(dbDataItem);
                    } catch (QueryError& err) {
                        this->rollback();  // rollback transaction
                        sqlite3_free(zErrMsg);
                        throw; // re-throw the same exception
                    }
                }

                this->commit(); // Commit transaction

                return newDbDataMap;
            }

            void update(const typeof T::data& dbData) {
                std::string sql = "UPDATE " + T::getTable() + " SET ";

                for (const auto& [key, value] : T::serialize(dbData)) {
                    sql += key + "=" + value + ",";
                }
                sql.pop_back();
                sql.append(" WHERE _id = " + std::to_string(dbData.id));

                char *zErrMsg = nullptr;
                int rc = sqlite3_exec(this->_con->session, sql.c_str(), NULL, NULL, &zErrMsg);

                if(rc != SQLITE_OK) {
                    const std::string errMsg(zErrMsg);
                    sqlite3_free(zErrMsg);
                    throw QueryError(errMsg);
                }
            }
            void update(const std::vector<typeof T::data>& dbDataList) {
            if (dbDataList.empty()) {
                return;
            }

            // Begin transaction
            char *zErrMsg = nullptr;
            this->begin();

            for (const auto &dbDataItem: dbDataList) {
                try {
                    this->update(dbDataItem);
                } catch (QueryError& err) {
                    this->rollback();  // rollback transaction
                    sqlite3_free(zErrMsg);
                    throw;  // re-throw the same exception
                }
            }

            this->commit(); // Commit transaction
        }
            template <typename... Args>
            void update(const Where& where, Args... args) {
                std::string sql = "UPDATE " + T::getTable() + " SET ";

                ((sql += std::string(args) + ","), ...);  // fold expression to concatenate all args
                sql.pop_back(); // remove the last comma

                if (!where.empty()) {
                    sql.append(" WHERE " + static_cast<std::string>(where));
                }

                char *zErrMsg = nullptr;
                int rc = sqlite3_exec(this->_con->session, sql.c_str(), NULL, NULL, &zErrMsg);

                if(rc != SQLITE_OK) {
                    const std::string errMsg(zErrMsg);
                    sqlite3_free(zErrMsg);
                    throw QueryError(errMsg);
                }
            }

            unsigned int count(const Where& where = WhereAny()) {
                std::string sql = "SELECT COUNT(*) FROM " + T::getTable();
                if (!where.empty()) {
                    sql.append(" WHERE " + static_cast<std::string>(where));
                }

                char *zErrMsg = nullptr;
                unsigned int rowCount;
                sqlite3_stmt *stmt;
                auto rc = sqlite3_prepare_v2(this->_con->session, sql.c_str(), -1, &stmt, 0 );

                if (rc == SQLITE_OK) {
                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        rowCount = sqlite3_column_int(stmt, 0);
                    }
                    sqlite3_finalize(stmt);
                }
                else {
                    std::string errMsg(zErrMsg);
                    sqlite3_free(zErrMsg);
                    throw QueryError(errMsg.c_str());
                }

                return rowCount;
            }

            unsigned int exists(const Where& where) {
                if (where.empty()) {
                    return false;
                }

                std::string sql = "SELECT exists(SELECT 1 FROM " + T::getTable() +
                                  " WHERE " + static_cast<std::string>(where) + ")";

                char *zErrMsg = nullptr;
                bool result;
                sqlite3_stmt *stmt;
                auto rc = sqlite3_prepare_v2(this->_con->session, sql.c_str(), -1, &stmt, 0 );

                if (rc == SQLITE_OK) {
                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        result = (bool)sqlite3_column_int(stmt, 0);
                    }
                    sqlite3_finalize(stmt);
                }
                else {
                    std::string errMsg(zErrMsg);
                    sqlite3_free(zErrMsg);
                    throw QueryError(errMsg.c_str());
                }

                return result;
            }
    };
}
#endif //SAMLIBINFO_DB_H
