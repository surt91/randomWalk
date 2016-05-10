#pragma once

#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>

#ifdef NLOG
// if compiled with -DNLOG, the compiler hopefully optimizes everything after the else away
#define LOG(level) \
    if(true) {} \
    else Logger(level)
#else
// http://stackoverflow.com/a/11826787/1698412
// low overhead macro
// prevents construction of the Logger object and evaluation of << operators
#define LOG(level) \
    if(level > Logger::verbosity) {} \
    else Logger(level)
#endif

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

/** Logs messages depending on a runtime set verbosity level.
 */
class Logger {
    public:
        Logger(log_level_t level)
            : level(level),
              ss()
        {
            ss.precision(12);
        }

        ~Logger()
        {
            if(level <= verbosity)
                std::cout << LABEL[level] << ss.str() << std::endl;
        }

        static int verbosity;

        template<class T>
        friend std::ostream& operator<<(Logger &&l, const T &obj);


    protected:
        log_level_t level;
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
    return os;
}
