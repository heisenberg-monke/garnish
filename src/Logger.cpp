#include "Logger.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>

namespace Garnish
{
    Logger::Logger()
        : m_out(&std::cout), m_log(nullptr), m_err(&std::cerr){}

    Logger &Logger::getLogger()
    {
        static Logger logger;
        return logger;
    }

    void Logger::setDebug(bool debug)
    {
        m_debug = debug;
        m_log = Stream(debug ? &std::clog : nullptr);
    }

    Stream &Logger::log()
    {
        m_log << "[INFO] ";
        return m_log;
    }

    Stream &Logger::display() {
        return m_out;
    }

    Stream &Logger::err()
    {
        m_err << "[ERROR] ";
        return m_err;
    }

    void Logger::beginProfile() {
        m_begin = Clock::now();
    }

    void Logger::endProfile(const std::string &label)
    {
        log() << label << ": " 
              << std::fixed << std::setprecision(6)
              << std::chrono::duration<double>(Clock::now() - m_begin).count() << " secs\n";
    }
}