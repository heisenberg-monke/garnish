#include "App.hpp"
#include "BPE.hpp"
#include "BpeToDot.hpp"
#include "FileSystem.hpp"
#include "Logger.hpp"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <iterator>
#include <thread>
#include <vector>

#include <format>

namespace Garnish
{
    void App::reportProgress(size_t iteration, const Tokens &inTokens, const Pairs &pairs) const
    {
        m_logger.log() << "Iteration: " << iteration << '\n'
                       << "       Text tokens count: " << inTokens.size() << '\n'
                       << "       BPE table size: " << pairs.size() << "\n\n";
    }

    App::App()
        : m_logger(Logger::getLogger()) {}

    void App::showHelp() const
    {
        m_logger.display() << "Usage: \n"
                           << "-td <input.bpe> <output.dot>\n"
                           << "-g <input.txt> [output.bpe] [output.tokens]\n"
                           << "-i <input.bpe>\n"
                           << "-h \n"
                           << "-d <training.bpe> <input.tokens> [output.txt]\n";
    }

    void App::generateBPE(const std::filesystem::path &input, const std::filesystem::path &outputBPE, const std::filesystem::path &outputTokens, size_t reportFreq) const
    {
        FileSystem &fs = FileSystem::getFS();
        std::string text;

        fs.readFile(input, text);

        Pairs pairs;
        Tokens inTokens;

        for(uint32_t i = 0; i < BPE::PRELUDE_SIZE; ++i)
            pairs.emplace_back(i, 0);

        inTokens.reserve(text.length());

        for(uint8_t c : text)
            inTokens.emplace_back(static_cast<uint32_t>(c));

        const size_t hardwareThreads = std::max<uint>(1, std::thread::hardware_concurrency());

        size_t iteration = 0;

        while(true)
        {
            if(++iteration % reportFreq == 0)
                reportProgress(iteration, inTokens, pairs);

            if(iteration % reportFreq == 0)
                m_logger.beginProfile();

            const size_t tokenCount = inTokens.size();

            if(tokenCount < 2)
                break;

            const size_t threadCount = std::min<size_t>(hardwareThreads, tokenCount - 1);

            std::vector<Freq> localFreqs(threadCount);
            std::vector<std::thread> workers;

            workers.reserve(threadCount);

            const size_t chunkSize = (tokenCount + threadCount - 2) / threadCount;

            for(size_t t = 0; t < threadCount; ++t)
            {
                const size_t begin = t * chunkSize;
                const size_t end = std::min(begin + chunkSize, tokenCount - 1);

                if(begin >= end)
                    continue;

                workers.emplace_back([&inTokens, &localFreqs, t, begin, end]()
                {
                    Freq &freq = localFreqs[t];

                    for(size_t i = begin; i < end; ++i)
                        ++freq[{inTokens[i], inTokens[i+1]}];
                });
            }

            for(auto &worker : workers)
                worker.join();

            Freq freq;

            for(const auto &localFreq : localFreqs)
            {
                for(const auto &[pair, count] : localFreq)
                    freq[pair] += count;
            }

            if(iteration % reportFreq == 0)
                m_logger.endProfile("Collecting stats");

            if(iteration % reportFreq == 0)
                m_logger.beginProfile();

            if(freq.empty())
                break;

            auto maxFreqIt = freq.begin();

            for(auto it = std::next(maxFreqIt); it != freq.end(); ++it)
            {
                if(it->second > maxFreqIt->second)
                    maxFreqIt = it;
            }

            const auto &key = maxFreqIt->first;
            const auto &value = maxFreqIt->second;

            if(iteration % reportFreq == 0)
                m_logger.endProfile("Finding most frequent pairs");

            if(value <= 1)
                break;

            const size_t newToken = static_cast<uint32_t>(pairs.size());

            pairs.emplace_back(key);

            if(iteration % reportFreq == 0)
                m_logger.beginProfile();

            const size_t replacementThreadCount = std::min(hardwareThreads, tokenCount);

            std::vector<Tokens> localOutputs(replacementThreadCount);

            workers.clear();
            workers.reserve(replacementThreadCount);

            const size_t replacementChunkSize = (tokenCount + replacementThreadCount - 1) / replacementThreadCount;
            
            for(size_t t = 0; t < replacementThreadCount; ++t)
            {
                const size_t begin = t * replacementChunkSize;
                const size_t end = std::min(begin + replacementChunkSize, tokenCount);

                if(begin >= end)
                    continue;

                workers.emplace_back([&inTokens, &localOutputs, &key, newToken, t, begin, end]()
                {
                    Tokens &out = localOutputs[t];
                    size_t i = begin;

                    out.reserve(end - begin);

                    if(i > 0 && i < inTokens.size() && Pair{inTokens[i-1], inTokens[i]} == key)
                        ++i;

                    while(i < end)
                    {
                        if(i + 1 < inTokens.size())
                        {
                            Pair pair{inTokens[i], inTokens[i+1]};

                            if(pair == key)
                            {
                                out.emplace_back(newToken);
                                i += 2;
                                continue;
                            }
                        }

                        out.emplace_back(inTokens[i++]);
                    }
                });
            }

            for(auto &worker : workers)
                worker.join();

            Tokens outTokens;
            size_t outputSize = 0;

            for(const auto &local : localOutputs)
                outputSize += local.size();

            outTokens.reserve(outputSize);

            for(auto &local : localOutputs)
            {
                outTokens.insert
                (
                    outTokens.end(),
                    std::make_move_iterator(local.begin()),
                    std::make_move_iterator(local.end())
                );
            }

            if(iteration % reportFreq == 0)
                m_logger.endProfile("Replacing the frequent pair");
                
            inTokens = std::move(outTokens);
        }

        reportProgress(iteration, inTokens, pairs);
        fs.writeFile(outputBPE, pairs.data(), pairs.size());
        fs.writeFile(outputTokens, inTokens.data(), inTokens.size());

        m_logger.log() << "Generated " << outputBPE << '\n';
        m_logger.log() << "Generated " << outputTokens << "\n";

        // FreqVec freqVec;

        // for(const auto &[key, value] : freq)
        //     freqVec.emplace_back(key, value);

        // std::sort(freqVec.begin(), freqVec.end(), [](const auto &a, const auto &b) {
        //     return a.right > b.right;
        // });

        // for(const auto &[key, value] : freqVec)
        //     m_logger.display() << '(' << key.left << ", " << key.right << ") => " << value << '\n';
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
            m_logger.display() << token << " => \"";

            buffer.clear();
            BPE::renderToken(pairs, token, buffer);

            for(char c : buffer)
            {
                if(c == '"')
                    m_logger.display() << "\\\"";

                else if(c == '\\')
                    m_logger.display() << "\\\\";

                else if(std::isprint(c))
                    m_logger.display() << c;

                else 
                {
                    m_logger.display() << "\\x" 
                                       << std::uppercase 
                                       << std::hex 
                                       << std::setw(2) 
                                       << std::setfill('0') 
                                       << static_cast<unsigned int>(static_cast<uint8_t>(c));
                }
            }

            m_logger.display() << "\"\n";
        }
    }

    void App::decodeTokens(const std::filesystem::path &inputBPE, const std::filesystem::path &inputTokens, const std::filesystem::path &output) const
    {
        auto &fs = FileSystem::getFS();

        Pairs pairs;
        std::string buffer;

        BPE::loadPairs(inputBPE, pairs, buffer);
        buffer.clear();

        fs.readFile(inputTokens, buffer);

        if(buffer.size() % sizeof(uint32_t) != 0)
            throw std::runtime_error(std::format("{}: File size in bytes ({}) should be divisible by {}", inputTokens.string(), buffer.size(), sizeof(uint32_t)));

        const size_t tokenCount = buffer.size() / sizeof(uint32_t);
        std::string text;

        for(size_t i = 0; i < tokenCount; ++i)
        {
            uint32_t token;

            std::memcpy(&token, buffer.data() + i * sizeof(uint32_t), sizeof(uint32_t));

            if(token >= pairs.size())
                throw std::runtime_error(std::format("{}: Token {} is outside the BPE table.", inputTokens.string(), token));

            BPE::renderToken(pairs, token, text);
        }

        fs.writeFile(output, text.data(), text.size());
        m_logger.log() << "Generated " << output << '\n';
    }
}