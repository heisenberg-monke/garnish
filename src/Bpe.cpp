#include "BPE.hpp"
#include "FileSystem.hpp"

#include <cstring>

#include <format>

namespace Garnish
{
    namespace BPE
    {
        void loadPairs(const std::filesystem::path &path, Pairs &pairs, std::string &buf)
        {
            FileSystem::getFS().readFile(path, buf);
            const auto pathStr = path.string();

            if(buf.size() % sizeof(Pair) != 0)
                throw std::runtime_error(pathStr + ": File size in bytes (" + std::to_string(buf.size()) + ") should be divisible by " + std::to_string(sizeof(Pair)));

            const size_t size = buf.size() / sizeof(Pair);

            if(size < PRELUDE_SIZE)
                throw std::runtime_error(std::format("{}: Pair count {} is too small. It must be at least {}", pathStr, size, PRELUDE_SIZE));

            pairs.resize(size);

            for(size_t i = 0; i < size; ++i)
            {
                std::memcpy(&pairs[i], buf.data() + i * sizeof(Pair), sizeof(Pair));

                if(i < PRELUDE_SIZE)
                {
                    if(pairs[i].left != i)
                        throw std::runtime_error(std::format("{}: pair {}: Left subtoken is {} instead of {}.", pathStr, i, pairs[i].left, i));

                    if(pairs[i].right != 0)
                        throw std::runtime_error(std::format("{}: pair {}: Right subtoken is {} instead of 0.", pathStr, i, pairs[i].right));
                }

                else 
                {
                    if(pairs[i].left >= size)
                        throw std::runtime_error(std::format("{}: pair {}: Left subtoken is {} >= {}", pathStr, i, pairs[i].left, size));

                    if(pairs[i].right >= size)
                        throw std::runtime_error(std::format("{}: pair {}: Right subtoken is {} >= {}", pathStr, i, pairs[i].right, size));
                }
            }
        }

        void renderToken(const Pairs &pairs, uint32_t token, std::string &out)
        {
            if(token == pairs[token].left)
            {
                out += static_cast<char>(token);
                return;
            }
            
            renderToken(pairs, pairs[token].left, out);
            renderToken(pairs, pairs[token].right, out);
        }
    }
}