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

        void reportProgress(size_t iteration, const Tokens &inTokens, const Pairs &pairs) const;

    public:
        App();

        void showHelp() const;
        void generateBPE(const std::filesystem::path &input, const std::filesystem::path &outputBPE, const std::filesystem::path &outputTokens, size_t reportFreq) const;
        void bpeToDot(const std::filesystem::path &input, const std::filesystem::path &output) const;
        void inspectBPE(const std::filesystem::path &input) const;
        void decodeTokens(const std::filesystem::path &inputBPE, const std::filesystem::path &inputTokens, const std::filesystem::path &output) const;
    };
}