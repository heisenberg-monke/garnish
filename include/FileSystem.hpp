#pragma once

#include <string>

#include <filesystem>

#include <fstream>

#include "Logger.hpp"

namespace Garnish
{
    class FileSystem
    {
        Logger &m_logger;

        FileSystem();
        FileSystem(const FileSystem &) = delete;
        FileSystem(FileSystem &&) = delete;

        FileSystem &operator=(const FileSystem &) = delete;
        FileSystem &operator=(FileSystem &&) = delete;

        std::filesystem::path resolveDir(const std::filesystem::path &path) const;

    public:
        static FileSystem &getFS();

        void readFile(const std::filesystem::path &path, std::string &out) const;

        template <typename T>
        void writeFile(const std::filesystem::path &path, const T *data, size_t size) const
        {
            auto filePath = resolveDir(path);
            auto filePathStr = filePath.string();

            std::ofstream file(filePath, std::ios::binary);

            if(!file)
                throw std::runtime_error("Failed to open file: " + filePathStr);

            file.write(reinterpret_cast<const char *>(data), static_cast<std::streamsize>(size * sizeof(T)));

            if(!file.good())
                throw std::runtime_error("Failed to write into file: " + filePathStr);
        }
    };
}