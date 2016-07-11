#include "Logging.hpp"

bool Logger::quiet = false;
int Logger::verbosity = 0;
std::string Logger::logfilename = "";

Logger::Logger(log_level_t level, std::string file, int line, std::string function)
    : level(level),
      ss(),
      file(file),
      line(line),
      function(function)
{
    ss.precision(12);
}

Logger::~Logger()
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

        if(!quiet)
            std::cout << ss.str() << std::endl;

        if(!logfilename.empty())
        {
            std::ofstream logfile(logfilename, std::ofstream::app);
            logfile << ss.str() << std::endl;
        }
    }
}
