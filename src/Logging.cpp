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
    if(level <= verbosity)
    {
        ss.precision(12);

        #ifdef __unix__
        if(isatty(fileno(stdout))) // Terminal -> use colors
            label = CLABEL[level];
        else
        #endif
            label = LABEL[level]; // Windows or file -> do not use color
    }
}

Logger::~Logger()
{
    if(level <= verbosity)
    {
        if(level <= LOG_WARNING)
            ss << " (" << file << ":" << line << " [" << function << "()]) ";

        if(!quiet)
            std::cout << label << ss.str() << std::endl;

        if(!logfilename.empty())
        {
            std::ofstream logfile(logfilename, std::ofstream::app);
            logfile << LABEL[level] << ss.str() << "\n";
        }
    }
}
