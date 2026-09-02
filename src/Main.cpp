#include <exception>
#include <filesystem>
#include <iostream>

#include <stdexcept>
#include <string>

// #include <algorithm>

#include "App.hpp"
#include "Logger.hpp"

enum class Subcommand
{
    NONE,
    HELP,
    TO_DOT,
    GENERATE,
    INSPECT,
    DECODE
};

// void renderTokens(const Pairs &pairs, const Tokens &tokens)
// {
//     for(size_t i = 0; i < tokens.size(); ++i)
//     {
//         uint32_t token = tokens[i];

//         if(tokens[i] >= pairs.size())
//             throw std::runtime_error("Invalid number of tokens.");

//         if(pairs[token].left == token)
//             std::cout << static_cast<char>(token);

//         else
//             std::cout << '[' << token << ']';
//     }

//     std::cout << '\n';
// }

int main(int argc, char **argv)
{
    try
    {
        if(argc <= 1)
            throw std::runtime_error("No subcommand given.");

        bool debug = false;

        Subcommand curr = Subcommand::NONE;

        std::filesystem::path input;
        std::filesystem::path input2;
        std::filesystem::path output;
        std::filesystem::path output2;

        size_t reportFreq = 100;

        auto setCurrSubCmd = [&curr](Subcommand cmd)
        {
            if(curr != Subcommand::NONE)
                throw std::runtime_error("Cannot glob subcommands.");

            curr = cmd;
        };

        for(int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];

            if(arg == "--debug")
            {
                debug = true;
                reportFreq = (i+1 >= argc || argv[i+1][0] == '-') ? 100 : std::stoul(argv[++i]);

                if(reportFreq == 0)
                    throw std::runtime_error("Report freq must be greater than 0.");
            }

            else if(arg == "-h")
                setCurrSubCmd(Subcommand::HELP);

            else if(arg == "-td")
            {
                setCurrSubCmd(Subcommand::TO_DOT);

                if(i+1 >= argc || argv[i+1][0] == '-')
                    throw std::runtime_error("No input file provided.");

                input = argv[++i];
                output = (i >= argc || argv[i][0] == '-') ? "pairs.dot" : argv[i];
            }

            else if(arg == "-g")
            {
                setCurrSubCmd(Subcommand::GENERATE);

                if(i+1 >= argc || argv[i+1][0] == '-')
                    throw std::runtime_error("No input file provided.");

                input = argv[++i];
                output = (i+1 >= argc || argv[i+1][0] == '-') ? "pairs.bpe" : argv[++i];
                output2 = (i+1 >= argc || argv[i+1][0] == '-') ? "tokens.tokens" : argv[++i];
            }

            else if(arg == "-i")
            {
                setCurrSubCmd(Subcommand::INSPECT);

                if(i+1 >= argc || argv[i+1][0] == '-')
                    throw std::runtime_error("No input file provided.");

                input = argv[++i];
            }

            else if(arg == "-d")
            {
                setCurrSubCmd(Subcommand::DECODE);

                if(i+1 >= argc || argv[i+1][0] == '-')
                    throw std::runtime_error("No input files provided.");

                input = argv[++i];

                if(i+1 >= argc || argv[i+1][0] == '-')
                    throw std::runtime_error("No input tokens provided.");

                input2 = argv[++i];
                output = (i+1 >= argc || argv[i+1][0] == '-') ? "output.txt" : argv[++i];
            }

            else
                throw std::runtime_error("Unknown subcommand: " + arg);
        }

        auto &logger = Garnish::Logger::getLogger();
        logger.setDebug(debug);

        Garnish::App app;

        switch(curr)
        {
            case Subcommand::NONE:
                throw std::runtime_error("No subcommand given.");

            case Subcommand::HELP:
                app.showHelp();
                break;

            case Subcommand::TO_DOT:
                app.bpeToDot(input, output);
                break;

            case Subcommand::GENERATE:
                app.generateBPE(input, output, output2, reportFreq);
                break;

            case Subcommand::INSPECT:
                app.inspectBPE(input);
                break;

            case Subcommand::DECODE:
                app.decodeTokens(input, input2, output);
                break;
        }
        
    }

    catch(const std::exception &e)
    {
        std::cerr << "[ERROR] " << e.what() << '\n';
        return 1;
    }

    return 0;
}