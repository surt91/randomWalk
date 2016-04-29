#include "stat.hpp"

/** Append a number to the rolling mean.
 */
double RollingMean::add(double x)
{
    ++count;
    count = std::min(count, state.size());
    state.pop_back();
    state.push_front(x);
    m_mean = std::accumulate(state.begin(), state.begin()+count, 0.0) / count;
    return m_mean;
}

/** Return the mean of the current window.
 */
double RollingMean::mean() const
{
    return m_mean;
}

/** Return the variance of the current window.
 */
double RollingMean::var(size_t last) const
{
    //~ double tmp = std::accumulate(state.begin(), state.begin()+count, 0.0, [&](double part, double next){return part + (m_mean - next) * (m_mean - next);});
    if(last <= 0)
        last = count;
    last = std::min(last, count);

    double tmp = 0;
    for(auto i = state.begin(); i != state.begin()+last; ++i)
        tmp += (m_mean - *i) * (m_mean - *i);
    return tmp/last;
}


/** Format time difference human readable.
 *
 * \param start is the timestamp at the beginning
 * \param end is the timestamp at the end
 * \return human readable time
 */
std::string time_diff(clock_t start, clock_t end)
{
    return std::to_string((double)(end - start) / CLOCKS_PER_SEC) + "s";
}

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

/** Yields the maximum vmem needed until the call to this function
 *
 * This function queries /proc/PID/status for VmPeak using a call
 * to grep. This is not very portable!
 *
 * \return string indicating VmPeak
 */
std::string vmPeak()
{
    std::string pid = std::to_string(getpid());
    std::string cmd("grep VmPeak /proc/"+pid+"/status");
    // or do I need "VmHWM" (high water mark)?
    return exec(cmd.c_str());
}
