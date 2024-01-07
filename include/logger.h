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


#ifndef SAMLIBINFO_LOGGER_H
#define SAMLIBINFO_LOGGER_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>

namespace logger {
    enum class LogLevel {
        Debug,
        Info,
        Warning,
        Error
    };

    struct LogEntry {
        std::chrono::system_clock::time_point time;
        LogLevel level;
        std::string message;
    };

    class ILogFilter {
        public:
            virtual ~ILogFilter() = default;
            virtual bool filter(const LogEntry& entry) = 0;
    };

    class ILogFormatter {
        public:
            virtual ~ILogFormatter() = default;
            virtual std::string format(const LogEntry& entry) = 0;
    };

    class MinimalLogLevelFilter : public ILogFilter {
        private:
            LogLevel _level;

        public:
            explicit MinimalLogLevelFilter(LogLevel level) : _level(level) {}
            [[nodiscard]] bool filter(const LogEntry& entry) override { return entry.level >= this->_level; }
    };


    class ISO8601LogFormatter : public ILogFormatter {
        public:
            std::string format(const LogEntry& entry) override;
    };


    class Logger {
        private:
            std::ostream* _os;
            std::vector<std::unique_ptr<ILogFilter>> _filters;
            std::unique_ptr<ILogFormatter> _formatter;

            friend struct LoggerStream;

        public:
            Logger(
                    std::ostream* os = &std::cout,
                    std::unique_ptr<ILogFormatter> formatter = std::make_unique<ISO8601LogFormatter>()
            );
            ~Logger() = default;

            void addFilter(std::unique_ptr<ILogFilter> filter);
            void setLogLevel(LogLevel level);

            struct LoggerStream {
                protected:
                    Logger* _logger;
                    std::ostringstream _ss;
                    LogLevel _level;

                    LogEntry _getEntry();
                    bool _isAvailable();
                    bool _isAvailable(const LogEntry& entry);

                public:
                    typedef std::basic_ostream<char, std::char_traits<char> > CoutType; // this is the type of std::cout
                    typedef CoutType& (*StandardEndLine)(CoutType&); // this is the function signature of std::endl

                    template<typename T>
                    LoggerStream& operator<<(const T& message);
                    LoggerStream& operator<<(const char* const& message); // handling a constant strings like "this one"
                    LoggerStream& operator<<(StandardEndLine handler);    // define an operator<< to take in std::endl

                    LoggerStream(Logger* logger, LogLevel level);
                    ~LoggerStream();
                    void flush();
                    void clear();
            };

            LoggerStream debug;
            LoggerStream info;
            LoggerStream warning;
            LoggerStream error;
    };
};
#endif //SAMLIBINFO_LOGGER_H
