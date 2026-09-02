#pragma once

#include "Logger.hpp"
#include "BPE.hpp"

#include <unordered_map>

#include <filesystem>

namespace Garnish
{
    struct PairHash
    {
        size_t operator()(const Pair &p) const noexcept
        {
            size_t h1 = std::hash<uint32_t>{}(p.left);
            size_t h2 = std::hash<uint32_t>{}(p.right);

            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };

    using Freq = std::unordered_map<Pair, size_t, PairHash>;
    using FreqVec = std::vector<std::pair<Pair, size_t>>;
    using Tokens = std::vector<uint32_t>;
    
    class App
    {
        Logger &m_logger;

    public:
        App();

        void showHelp() const;
        void generateBPE(const std::filesystem::path &input, const std::filesystem::path &output) const;
        void bpeToDot(const std::filesystem::path &input, const std::filesystem::path &output) const;
        void inspectBPE(const std::filesystem::path &input) const;
    };
}