#pragma once

#include <sstream>
#include <iostream>
#include <vector>

// http://stackoverflow.com/questions/1255576/what-is-good-practice-for-generating-verbose-output

enum log_level_t {
    LOG_QUIET = 0,
    LOG_ALWAYS,
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_TIMING,
    LOG_DEBUG,
    LOG_ALL,
    LOG_TOO_MUCH
};

static const std::vector<std::string> LABEL = {
    "",
    "",
    "\x1B[31mError:   \033[0m",
    "\x1B[33mWarning: \033[0m",
    "\x1B[34mInfo:    \033[0m",
    "\x1B[34mTiming:  \033[0m",
    "\x1B[34mDebug:   \033[0m",
    "\x1B[34mDebug2:  \033[0m",
    "\x1B[34mDebug3:  \033[0m"
};

class Logger {
    public:
        Logger(log_level_t level)
            : level(level)
        {
            ss.precision(12);
        }

        ~Logger()
        {
            // VERBOSITY_LEVEL is a global variable and could be changed at runtime
            if(level <= verbosity)
                std::cout << LABEL[level] << ss.str() << std::endl;
        }

        static int verbosity;

        template<class T>
        friend std::ostream& operator<<(Logger &&l, const T &obj);
        

    protected:
        log_level_t level;
        std::string msg;
        std::stringstream ss;
};

template<class T>
std::ostream& operator<<(Logger &&l, const T &obj)
{
    l.ss << " " << obj;
    return l.ss;
}

template<class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T> &v)
{
    for(auto i : v)
        os << i << " ";
    os << "\n";
    return os;
}

