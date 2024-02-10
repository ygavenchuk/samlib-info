//
// Created by Yurii Havenchuk on 13.01.2024.
//

#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include "fs.h"

using namespace fs;


bool fs::path::exists(const std::string& path) {
    std::filesystem::path fsPath(path);
    try {
        return std::filesystem::exists(fsPath);
    }
    catch (std::filesystem::filesystem_error& err) {
        throw FSError(std::string("Cannot check path existence: ") + err.what());
    }
}

bool fs::path::isDirectory(const std::string &path) {
    std::filesystem::path fsPath(path);
    return std::filesystem::is_directory(fsPath);
}

bool fs::path::isWriteable(const std::string& filePath) {
    return access(filePath.c_str(), W_OK) == 0;
}

std::filesystem::path fs::path::resolve(const std::string& path) {
    try {
        if (path.starts_with("~")) { // `filesystem::absolute()` cannot resolve `~`.
            return std::filesystem::absolute(static_cast<std::string>(std::getenv("HOME")) + path.substr(1));
        }
        return std::filesystem::absolute(path);
    }
    catch (std::filesystem::filesystem_error& err) {
        throw FSError("Cannot resolve path due to \"" + static_cast<std::string>(err.what()) + "\"");
    }
}

std::filesystem::path fs::path::ensure(const std::string &path, bool stripFileName) {
    auto directory = fs::path::resolve(path);
    auto fileName = directory.filename();

    if (stripFileName) {
        directory.remove_filename();
    }

    if (fs::path::exists(directory)) {
        return stripFileName? directory / fileName : directory;
    }

    try {
        if (std::filesystem::create_directories(directory)) {
            throw FSError("Cannot create directories for the path \"" + directory.string() + "\"");
        }
    }
    catch (std::filesystem::filesystem_error& err) {
        throw FSError("Cannot create directories for the path \"" + directory.string() + "\"" + err.what());
    }

    return stripFileName? directory / fileName : directory;
}


BookStorage::BookStorage(const std::string& location){
    if (location.empty()) {
        throw FSError("The location cannot be empty");
    }

    try {
        this->_location = fs::path::resolve(location) / "books";
    }
    catch (std::filesystem::filesystem_error& err) {
        throw FSError("Invalid location: " + std::string(err.what()));
    }
}

std::string BookStorage::_getFullPath(std::string& bookUrl, BookType bookType) const {
    if (bookUrl.empty()) {
        throw FSError("Invalid path argument(s)");
    }

    if constexpr (path::SEPARATOR != '/') {
        std::replace(bookUrl.begin(), bookUrl.end(), '/', path::SEPARATOR);
    }

    if (bookUrl.starts_with(path::SEPARATOR)) {
        bookUrl = bookUrl.substr(1);
    }

    return this->_location / (bookUrl + (bookType == BookType::FB2 ? ".fb2.zip" : ".html") );
}

std::string BookStorage::ensurePath(std::string& bookUrl, BookType bookType) const {
    return fs::path::ensure(this->_getFullPath(bookUrl, bookType));
}

bool BookStorage::exists(std::string &bookUrl, fs::BookType bookType) const {
    return fs::path::exists(this->_getFullPath(bookUrl, bookType));
}

std::string BookStorage::getFullPathIfExists(const std::string& bookUrl) const {
    auto bookUrlCopy = bookUrl;

    if (this->exists(bookUrlCopy, fs::BookType::FB2)) {
        return this->ensurePath(bookUrlCopy, fs::BookType::FB2);
    }

    if (this->exists(bookUrlCopy, fs::BookType::HTML)) {
        return this->ensurePath(bookUrlCopy, fs::BookType::HTML);
    }

    return std::string{};
}