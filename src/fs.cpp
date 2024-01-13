//
// Created by Yurii Havenchuk on 13.01.2024.
//

#include <filesystem>
#include <unistd.h>
#include <algorithm>
#include <utility>
#include "fs.h"

using namespace fs;

FSStorage::FSStorage(const std::string& location, std::string appName) : _appName(std::move(appName)) {
    if (this->_appName.empty()) {
        throw FSError("The location cannot be empty");
    }

    if (location.empty()) {
        throw FSError("The location cannot be empty");
    }

    try {
        this->_location = FSStorage::_resolvePath(location);
    }
    catch (std::filesystem::filesystem_error& err) {
        throw FSError("Invalid location: " + std::string(err.what()));
    }

    if (!FSStorage::_isDirectory(this->_location)) {
        throw FSError("Location should be a directory!");
    }

    if (!FSStorage::_isWriteable(this->_location)) {
        throw FSError("Location should be writeable");
    }

    if (!this->_location.ends_with(std::filesystem::path::preferred_separator)) {
        this->_location += std::filesystem::path::preferred_separator;
    }

    this->_location += this->_appName + std::filesystem::path::preferred_separator
                       + "books" + std::filesystem::path::preferred_separator;
}

bool FSStorage::_isPathExists(const std::string& path) {
    std::filesystem::path fsPath(path);
    return std::filesystem::exists(fsPath);
}

bool FSStorage::_isDirectory(const std::string &path) {
    std::filesystem::path fsPath(path);
    return std::filesystem::is_directory(fsPath);
}

bool FSStorage::_isWriteable(const std::string& filePath) {
    return access(filePath.c_str(), W_OK) == 0;
}

std::string FSStorage::_resolvePath(const std::string& path) {
    try {
        return std::filesystem::canonical(path).generic_string();
    }
    catch (std::filesystem::filesystem_error& err) {
        throw FSError("Cannot resolve path due to \"" + static_cast<std::string>(err.what()) + "\"");
    }
}

std::string FSStorage::_getFullPath(std::string& bookUrl,  BookType bookType) const {
    if (bookUrl.empty()) {
        throw FSError("Invalid path argument(s)");
    }

    if constexpr (std::filesystem::path::preferred_separator != '/') {
        std::replace(
            bookUrl.begin(), bookUrl.end(), '/', std::filesystem::path::preferred_separator
        );
    }

    if (bookUrl.starts_with(std::filesystem::path::preferred_separator)) {
        bookUrl = bookUrl.substr(1);
    }

    return this->_location + bookUrl + (bookType == BookType::FB2 ? ".fb2.zip" : ".html") ;
}


std::string FSStorage::ensurePath(std::string& bookUrl) const {
    auto fullPath = this->_getFullPath(bookUrl);
    auto directory = std::filesystem::path(fullPath).remove_filename();

    if (!FSStorage::_isPathExists(directory)) {
        try {
            if (std::filesystem::create_directories(directory)) {
                throw FSError("Cannot create directories for the \"" + bookUrl + "\"");
            }
        }
        catch (std::filesystem::filesystem_error& err) {
            throw FSError("Cannot create directories for the \"" + bookUrl + "\": " + err.what());
        }
    }

    return fullPath;
}

