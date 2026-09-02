#pragma once

#include <cstdint>

#include <string>
#include <vector>

#include <filesystem>

namespace Garnish
{
    struct Pair
    {
        uint32_t left;
        uint32_t right;

        bool operator==(const Pair &p) const {
            return left == p.left && right == p.right;
        }
    };

    using Pairs = std::vector<Pair>;

    namespace BPE
    {
        void loadPairs(const std::filesystem::path &path, Pairs &pairs, std::string &buf);
    }
}