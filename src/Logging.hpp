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
        Logger(log_level_t level, const std::string &msg)
            : level(level),
              msg(msg)
        {}

        ~Logger()
        {
            // VERBOSITY_LEVEL is a global variable and could be changed at runtime
            if(level <= verbosity)
                std::cout << LABEL[level] << msg << std::endl;
        }

        static int verbosity;

        template<class T>
        friend Logger& operator<<(Logger &&l, const T &obj);

    protected:
        log_level_t level;
        std::string msg;
};

template<class T>
Logger& operator<<(Logger &&l, const T &obj)
{
    if(l.level > l.verbosity)
        return l;

    std::stringstream ss;
    ss << " " << obj;
    l.msg += ss.str();
    return l;
}

// Helper function. Class Logger will not be used directly.
template<log_level_t level>
Logger log(const std::string &msg = std::string())
{
    return Logger(level, msg);
}
