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


#ifndef SAMLIBINFO_MINER_H
#define SAMLIBINFO_MINER_H

#include <string>
#include "db.h"
#include "http.h"
#include "parser.h"
#include "logger.h"
#include "errors.h"


namespace miner {
//    http::Settings Settings = http::Settings {"http", "samlib.ru"};
//    const std::vector<std::string> KnownDomains{"samlib.ru", "zhurnal.lib.ru"};
    const auto AUTHOR_URL_PATTERN =
          R"lit(^(?:http:\/\/(?:(?:samlib\.ru)|(?:zhurnal\.lib\.ru)))?)lit" // may contain domain
          R"lit((\/?([a-z])\/\2[a-z0-9_-]+\/?).*$)lit" // can be in full form (i.e. /l/lorem_ipsum)
          R"lit(|^([a-z0-9-_]+\/?)$)lit" // or just contain meaningful part of the url
    ;
    const std::string S_PROTOCOL = "http";
    const std::string S_DOMAIN = "samlib.ru";

    class MinerError : public SamLibError {
        public:
            explicit MinerError(const std::string& arg) : SamLibError("MinerError: " + arg) {}
            explicit MinerError(const char* arg) : SamLibError(std::string("MinerError: ") + arg) {}
    };

    class InvalidURL : public MinerError {
        public:
            explicit InvalidURL(const std::string& arg) : MinerError("InvalidURL: " + arg) {}
            explicit InvalidURL(const char* arg) : MinerError(std::string("InvalidURL: ") + arg) {}
    };

    class AuthorNotFound : public MinerError {
        public:
            explicit AuthorNotFound(const std::string& arg) : MinerError("AuthorNotFound: " + arg) {}
            explicit AuthorNotFound(const char* arg) : MinerError(std::string("AuthorNotFound: ") + arg) {}
    };

    struct Changes {
        db::Books books;
        db::GroupBooks groups;
        [[nodiscard]] bool empty() const {return books.empty() && groups.empty();}
    };

    struct Difference {
        Changes added;
        Changes updated;
        Changes removed;
        bool isPageRemoved;

        Difference() : isPageRemoved(false) {}
        [[nodiscard]] bool empty() const {return added.empty() && updated.empty() && removed.empty() && !isPageRemoved;}
    };

    class Miner {
        private:
            const std::shared_ptr<logger::Logger> _logger;
            const std::shared_ptr<db::Connection> _con;
            const std::shared_ptr<db::DB<db::Book>> _tBook;
            const std::shared_ptr<db::DB<db::GroupBook>> _tGroup;
            const std::shared_ptr<db::DB<db::Author>> _tAuthor;

            void _logDiff(const Difference& diff, const db::AuthorData& author);
            std::string _getAuthorUrl(const std::string& url) const;

        public:
            Miner(const std::shared_ptr<db::Connection>& connection, const std::shared_ptr<logger::Logger>& logger);
            Miner(
                    const std::shared_ptr<db::Connection>& connection,
                    const std::shared_ptr<logger::Logger>& logger,
                    const std::shared_ptr<db::DB<db::Author>>& authorDB,
                    const std::shared_ptr<db::DB<db::GroupBook>>& groupDB,
                    const std::shared_ptr<db::DB<db::Book>>& bookDB
                  );
            ~Miner() = default;

            db::AuthorData getAuthor(const std::string& url) const;
            Difference getUpdates(const db::AuthorData& author);
            void apply(Difference& diff, db::AuthorData& author);
            void sync(db::AuthorData& author);
            void syncAll(
                const std::function<void(const db::AuthorData&, unsigned int current, unsigned int total)>& progressCallback
            );
            void syncAll();
        };
}

#endif //SAMLIBINFO_MINER_H