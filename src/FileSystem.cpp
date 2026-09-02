#include "FileSystem.hpp"

#include <fstream>
#include <ios>
#include <stdexcept>

namespace Garnish
{
    FileSystem::FileSystem()
        : m_logger(Logger::getLogger()) {}

    std::filesystem::path FileSystem::resolveDir(const std::filesystem::path &path) const
    {
        if(path.is_absolute())
            return path;

        return PROJECT_ROOT / path;
    }

    FileSystem &FileSystem::getFS()
    {
        static FileSystem fs;
        return fs;
    }

    void FileSystem::readFile(const std::filesystem::path &path, std::string &out) const
    {
        auto filePath = resolveDir(path);
        auto filePathStr = filePath.string();

        std::ifstream file(filePath, std::ios::binary);

        if(!file)
            throw std::runtime_error("Failed to open file: " + filePathStr);

        auto fileSize = std::filesystem::file_size(filePath);
        auto strSize = out.size();

        out.resize(strSize + fileSize);
        file.read(out.data() + strSize, fileSize);

        if(!file.good())
            throw std::runtime_error("Failed to read file: " + filePathStr);
    }
}