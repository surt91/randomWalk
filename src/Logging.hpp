#pragma once

#ifdef __unix__
#include <cstdio>
#include <unistd.h>
#endif

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
    else Logger(level, __FILE__, __LINE__, __func__)
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
    "Error:   ",
    "Warning: ",
    "Info:    ",
    "Timing:  ",
    "Debug:   ",
    "Debug2:  ",
    "Debug3:  "
};

static const std::vector<std::string> CLABEL = {
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
        Logger(log_level_t level, std::string file="", int line=0, std::string function="")
            : level(level),
              ss(),
              file(file),
              line(line),
              function(function)
        {
            ss.precision(12);
        }

        ~Logger()
        {
            if(level <= verbosity)
            {
                #ifdef __unix__
                if(isatty(fileno(stdout))) // Terminal, use colors
                    std::cout << CLABEL[level];
                else
                #endif
                    std::cout << LABEL[level];

                if(level <= LOG_WARNING)
                    ss << " (" << file << ":" << line << " [" << function << "()]) ";

                std::cout << ss.str() << std::endl;
            }
        }

        static int verbosity;

        template<class T>
        friend std::ostream& operator<<(Logger &&l, const T &obj);


    protected:
        log_level_t level;
        std::stringstream ss;

        std::string file;
        int line;
        std::string function;
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
        os << i << ", ";
    return os;
}
