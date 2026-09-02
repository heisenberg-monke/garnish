#include "BPE.hpp"
#include "FileSystem.hpp"

#include <cstring>

namespace Garnish
{
    namespace BPE
    {
        void loadPairs(const std::filesystem::path &path, Pairs &pairs, std::string &buf)
        {
            FileSystem::getFS().readFile(path, buf);

            if(buf.size() % sizeof(Pair) != 0)
                throw std::runtime_error("Size of " + path.string() + " (" + std::to_string(buf.size()) + ") should be divisible by " + std::to_string(sizeof(Pair)));

            const size_t size = buf.size() / sizeof(Pair);
            pairs.resize(size);

            for(size_t i = 0; i < size; ++i)
                std::memcpy(&pairs[i], buf.data() + i * sizeof(Pair), sizeof(Pair));
        }
    }
}