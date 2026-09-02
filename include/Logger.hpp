#pragma once

#include <iostream>

#include <string>

#include <chrono>

namespace Garnish
{
    class Stream
    {
        std::ostream *m_stream;

    public:
        explicit Stream(std::ostream *stream)
            : m_stream(stream) {}

        template <typename T>
        Stream &operator<<(const T &value)
        {
            if(m_stream)
                *m_stream << value;

            return *this;
        }

        Stream& operator<<(std::ostream& (*manip)(std::ostream&))
        {
            if(m_stream)
                *m_stream << manip;

            return *this;
        }

    };

    using Clock = std::chrono::steady_clock;

    class Logger
    {
        bool m_debug = false;

        Stream m_out;
        Stream m_err;
        Stream m_log;

        Clock::time_point m_begin;
    
        Logger();
        Logger(const Logger &) = delete;
        Logger(Logger &&) = delete;

        Logger &operator=(const Logger &) = delete;
        Logger &operator=(Logger &&) = delete;

    public:
        static Logger &getLogger();

        void setDebug(bool debug);

        Stream &log();
        Stream &display();
        Stream &err();

        void beginProfile();
        void endProfile(const std::string &label);
    };
}