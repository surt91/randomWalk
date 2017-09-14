#include "runtime.hpp"

/** Format time difference human readable.
 *
 * \param start is the timestamp at the beginning
 * \param end is the timestamp at the end
 * \param op is the number of operations to divide through
 * \return human readable time (per operation)
 */
std::string time_diff(clock_t start, clock_t end, int op)
{
    return std::to_string((double)(end - start) / CLOCKS_PER_SEC / (double) op) + "s";
}

#ifdef __unix__
/** Executes the command and returns its standard out.
 *
 * taken from http://stackoverflow.com/a/478960/1698412
 *
 * \param cmd is the shell command to be executed
 * \return standard output of the command
 */
std::string exec(const char* cmd)
{
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
        return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}
#endif

/** Yields the maximum vmem needed until the call to this function
 *
 * This function queries /proc/PID/status for VmPeak using a call
 * to grep. This is not very portable!
 *
 * \return string indicating VmPeak
 */
std::string vmPeak()
{
    #ifdef __unix__
    std::string pid = std::to_string(getpid());
    std::string cmd("grep VmPeak /proc/"+pid+"/status");
    // or do I need "VmHWM" (high water mark)?
    return exec(cmd.c_str());
    #else
    return "Not implemented for this platform";
    #endif
}
