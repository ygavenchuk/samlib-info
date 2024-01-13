//
// Created by Yurii Havenchuk on 13.01.2024.
//

#ifndef SAMLIBINFO_FS_H
#define SAMLIBINFO_FS_H

#include <string>
#include "errors.h"

namespace fs {
    class FSError : public SamLibError {
    public:
        explicit FSError(const std::string& arg) : SamLibError("FSError: " + arg) {}
        explicit FSError(const char* arg) : SamLibError(std::string("FSError: ") + arg) {}
    };

    enum class BookType {
        FB2,
        HTML
    };

    class FSStorage {
        private:
            std::string _location;
            std::string _appName;
            static bool _isPathExists(const std::string& path);
            static bool _isDirectory(const std::string& path);
            static bool _isWriteable(const std::string& path);
            static std::string _resolvePath(const std::string& path);
            std::string _getFullPath(std::string& bookUrl, BookType bookType=BookType::FB2) const;

        public:
            explicit FSStorage(const std::string& location, std::string appName = "SamLib");
            FSStorage(const FSStorage& other)
                    : _location(other._location),
                      _appName(other._appName) {}
            FSStorage(FSStorage&& other) noexcept
                    : _location(std::move(other._location)),
                      _appName(std::move(other._appName)) {}
            FSStorage& operator=(const FSStorage& other) {
                if (this != &other) {
                    _location = other._location;
                    _appName = other._appName;
                }
                return *this;
            }
            FSStorage& operator=(FSStorage&& other) noexcept {
                if (this != &other) {
                    _location = std::move(other._location);
                    _appName = std::move(other._appName);
                }
                return *this;
            }
            ~FSStorage() = default;

            std::string ensurePath(std::string& bookUrl) const;
    };


}

#endif //SAMLIBINFO_FS_H
