#include "App.hpp"
#include "BPE.hpp"
#include "BpeToDot.hpp"
#include "FileSystem.hpp"

namespace Garnish
{
    static inline void renderToken(const Pairs &pairs, uint32_t token, std::string &out)
    {
        if(token == pairs[token].left)
        {
            out += static_cast<char>(token);
            return;
        }
        
        renderToken(pairs, pairs[token].left, out);
        renderToken(pairs, pairs[token].right, out);
    }

    App::App()
        : m_logger(Logger::getLogger()) {}

    void App::showHelp() const
    {
        m_logger.display() << "Usage: \n"
                           << "-td <input.bpe> <output.dot>\n"
                           << "-g <input.txt> [output.bpe]\n";
    }

    void App::generateBPE(const std::filesystem::path &input, const std::filesystem::path &output) const
    {
        FileSystem &fs = FileSystem::getFS();
        std::string text;

        fs.readFile(input, text);

        Pairs pairs;
        Tokens inTokens;

        for(uint32_t i = 0; i < 256; ++i)
            pairs.emplace_back(i, 0);

        inTokens.reserve(text.length());

        for(auto c : text)
            inTokens.emplace_back(static_cast<uint32_t>(c));

        while(true)
        {
            Freq freq;
            Tokens outTokens;

            for(size_t i = 0; i+1 < inTokens.size(); ++i)
                ++freq[{inTokens[i], inTokens[i+1]}];

            auto maxFreqIt = freq.begin();

            for(auto it = maxFreqIt; it != freq.end(); ++it)
            {
                if(it->second > maxFreqIt->second)
                    maxFreqIt = it;
            }

            auto &key = maxFreqIt->first;
            auto &value = maxFreqIt->second;

            if(value <= 1)
                break;

            pairs.emplace_back(key);
            {
                size_t i = 0;

                while(i < inTokens.size())
                {
                    if(i + 1 >= inTokens.size())
                        outTokens.emplace_back(inTokens[i++]);

                    else  
                    {
                        Pair pair(inTokens[i], inTokens[i+1]);

                        if(pair == key)
                        {
                            outTokens.emplace_back(pairs.size() - 1);
                            ++i;
                        }

                        else 
                            outTokens.emplace_back(pair.left);

                        ++i;
                    }
                }
            }

            inTokens = outTokens;
        }

        FileSystem::getFS().writeFile(output, pairs.data(), pairs.size());

        m_logger.log() << "Generated " << output << '\n';

        // FreqVec freqVec;

        // for(const auto &[key, value] : freq)
        //     freqVec.emplace_back(key, value);

        // std::sort(freqVec.begin(), freqVec.end(), [](const auto &a, const auto &b) {
        //     return a.right > b.right;
        // });

        // for(const auto &[key, value] : freqVec)
        //     std::cout << '(' << key.left << ", " << key.right << ") => " << value << '\n';
    }

    void App::bpeToDot(const std::filesystem::path &input, const std::filesystem::path &output) const
    {
        Pairs pairs;
        std::string str;

        BPE::loadPairs(input, pairs, str);
        str.clear();
        
        BpeToDot::renderDot(pairs, str);
        FileSystem::getFS().writeFile(output, str.data(), str.size());

        m_logger.log() << "Generated " << output << '\n';
    }

    void App::inspectBPE(const std::filesystem::path &input) const
    {
        Pairs pairs;
        std::string buffer;

        BPE::loadPairs(input, pairs, buffer);
        
        for(uint32_t token = 0; token < pairs.size(); ++token)
        {
            buffer.clear();
            renderToken(pairs, token, buffer);

            m_logger.display() << token << " => " << buffer << '\n';
        }
    }
}