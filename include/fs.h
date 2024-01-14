//
// Created by Yurii Havenchuk on 13.01.2024.
//

#ifndef SAMLIBINFO_FS_H
#define SAMLIBINFO_FS_H

#include <filesystem>
#include <string>
#include "errors.h"

namespace fs {
    class FSError : public SamLibError {
    public:
        explicit FSError(const std::string& arg) : SamLibError("FSError: " + arg) {}
        explicit FSError(const char* arg) : SamLibError(std::string("FSError: ") + arg) {}
    };

    namespace path {
        const char SEPARATOR = std::filesystem::path::preferred_separator;

        /**
         * @brief Checks if a file or directory exists at the specified path.
         *
         * This function checks if a file or directory exists at the specified path.
         *
         * @param path The path to the file or directory to check.
         *
         * @throw FSError
         *
         * @return True if the file or directory exists, false otherwise.
         */
        bool exists(const std::string& path);

        /**
         * @brief Checks if the given path is a directory.
         *
         * This function checks if the given path is a valid directory or not.
         *
         * @param path The path to be checked.
         *
         * @return true if the path is a directory, false otherwise.
         */
        bool isDirectory(const std::string& path);

        /**
         * @brief Checks if a file or directory at the given path is writable.
         *
         * This function checks if the file or directory at the given path is writable.
         *
         * @param path The path of the file or directory to check.
         *
         * @return true if the file or directory is writable, false otherwise.
         */
        bool isWriteable(const std::string& path);

        /**
         * @brief Resolves the given path.
         *
         * This function takes a path as input and resolves it by removing any
         * relative components and symbolic links. The resolved path is returned.
         * If the input path is already an absolute path, then it is returned as is.
         *
         * @param path The path to be resolved.
         *
         * @throw FSError
         *
         * @return The resolved path.
         */
        std::filesystem::path resolve(const std::string& path);

        /**
         * @brief Ensures that the provided path exists by creating any missing directories.
         *
         * This function takes a path as input and checks if it exists. If the path does not exist, this function
         * creates all the missing directories in the path recursively. By default, this function assumes that the last
         * element in the path is a file and strips it off when creating directories. However, this behavior can be
         * disabled by setting the second parameter to false.
         *
         * @note The function only creates directories in the path and does not create any files.
         *
         * @param path The path to ensure.
         * @param stripFileName Set to true (default) if the function should strip the last element from the path.
         *                      Set to false if the path represents a directory.
         *
         * @throw FSError
         *
         * @return resolved (absolute) path.
         * @see fs::path::resolve()
         */
        std::filesystem::path ensure(const std::string& path, bool stripFileName=true);
    }

    enum class BookType {
        FB2,
        HTML
    };

    /**
    * @class BookStorage
    * @brief Class for managing a book storage
    *
    * This class provides functionality for managing a book storage. It allows you to specify the location of the
     * storage and perform operations such as ensuring the path of a book URL.
    */
    class BookStorage {
        private:
            std::filesystem::path _location;

        /**
         * @brief Get the full path of a given book URL.
         *
         * This function returns the full path of a given book URL based on the specified book type.
         *
         * @param bookUrl The URL of the book.
         * @param bookType The type of the book (optional, default value is BookType::FB2).
         *
         * @throw FSError (e.g. if `bookUrl` is empty)
         *
         * @return The full path of the book URL.
         *
         * @see BookType
         */
        std::string _getFullPath(std::string& bookUrl, BookType bookType=BookType::FB2) const;

        public:
            explicit BookStorage(const std::string& location);
            BookStorage(const BookStorage& other) : _location(other._location) {}
            BookStorage(BookStorage&& other) noexcept : _location(std::move(other._location)) {}
            BookStorage& operator=(const BookStorage& other) {
                if (this != &other) {
                    _location = other._location;
                }
                return *this;
            }
            BookStorage& operator=(BookStorage&& other) noexcept {
                if (this != &other) {
                    _location = std::move(other._location);
                }
                return *this;
            }
            ~BookStorage() = default;

        /**
         * @brief Ensures the existence of the file path specified by bookUrl.
         *
         * This function checks if the file path specified by bookUrl exists. If the file
         * path does not exist, the function attempts to create the necessary directories
         * to ensure the path is valid.
         *
         * @param bookUrl The book's URL that is converted to filesystem path and the path is created if necessary.
         * @param bookType The type of the book (optional, default value is BookType::FB2).
         *
         * @throw FSError
         *
         * @return resolved (absolute) path.
         * @see fs::path::resolve()
         */
        std::string ensurePath(std::string& bookUrl, BookType bookType = BookType::FB2) const;
    };
}

#endif //SAMLIBINFO_FS_H
