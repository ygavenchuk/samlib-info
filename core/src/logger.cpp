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


#include <iomanip>  // for std::put_time
#include "logger.h"

using namespace logger;

std::string ISO8601LogFormatter::format(const LogEntry& entry) {
    auto time_t = std::chrono::system_clock::to_time_t(entry.time);
    auto duration = entry.time.time_since_epoch();
    auto milliseconds = duration_cast<std::chrono::milliseconds>(duration).count() % 1000; // get milliseconds only

    std::stringstream stream;
    stream << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%dT%H:%M:%S")
           << "." << std::setw(3) << std::setfill('0') << milliseconds << "]"
    ;

    // Add logger level prefix to the stream
    switch(entry.level) {
        case LogLevel::Debug:   stream << " [DEBUG] "; break;
        case LogLevel::Info:    stream << " [INFO] "; break;
        case LogLevel::Warning: stream << " [WARNING] "; break;
        case LogLevel::Error:   stream << " [ERROR] "; break;
    }

    stream << entry.message;

    return stream.str();
}

Logger::Logger(std::ostream* os, std::unique_ptr<ILogFormatter> formatter) :
    _os(os),
    _formatter(std::move(formatter)),
    debug{this, LogLevel::Debug},
    info{this, LogLevel::Info},
    warning{this, LogLevel::Warning},
    error{this, LogLevel::Error}
{}

void Logger::addFilter(std::unique_ptr<ILogFilter> filter) {
    this->_filters.push_back(std::move(filter));
}

void Logger::setLogLevel(logger::LogLevel level) {
    this->addFilter(std::make_unique<MinimalLogLevelFilter>(level));
}

LogEntry Logger::LoggerStream::_getEntry() {
    return LogEntry{std::chrono::system_clock::now(), this->_level, this->_ss.str()};
}

bool Logger::LoggerStream::_isAvailable(const LogEntry& entry) {
    if (this->_logger->_filters.empty()) {
        return !entry.message.empty();
    }

    for (const auto& _filter : this->_logger->_filters) {
        if(entry.message.empty() || !_filter->filter(entry)) {
            return false;
        }
    }

    return true;
}

bool Logger::LoggerStream::_isAvailable() { return this->_isAvailable(this->_getEntry()); }

template<typename T>
Logger::LoggerStream& Logger::LoggerStream::operator<<(const T& message) {
    this->_ss << message;
    return *this;
}

Logger::LoggerStream& Logger::LoggerStream::operator<<(const char* const& message) {
    this->_ss << message;
    return *this;
}

Logger::LoggerStream& Logger::LoggerStream::operator<<(StandardEndLine handler) {
    if (this->_isAvailable()) {
        this->flush();
        handler(*this->_logger->_os);
    }
    return *this;
}

Logger::LoggerStream::LoggerStream(Logger* logger, LogLevel level) : _logger(logger), _level(level) {};

void Logger::LoggerStream::clear() {
    this->_ss.str("");
    this->_ss.clear();
}

void Logger::LoggerStream::flush() {
    *(this->_logger->_os) << this->_logger->_formatter->format(this->_getEntry());
    this->clear();
}

Logger::LoggerStream::~LoggerStream() {
    if (this->_isAvailable()) {
        this->flush();
    }
}

template Logger::LoggerStream& Logger::LoggerStream::operator<< <std::string>(const std::string& message);
template Logger::LoggerStream& Logger::LoggerStream::operator<< <int>(const int& message);
template Logger::LoggerStream& Logger::LoggerStream::operator<< <unsigned int>(const unsigned int& message);
template Logger::LoggerStream& Logger::LoggerStream::operator<< <bool>(const bool& message);
template Logger::LoggerStream& Logger::LoggerStream::operator<< <float>(const float& message);
template Logger::LoggerStream& Logger::LoggerStream::operator<< <double>(const double& message);
template Logger::LoggerStream& Logger::LoggerStream::operator<< <long>(const long& message);
template Logger::LoggerStream& Logger::LoggerStream::operator<< <unsigned long>(const unsigned long& message);
