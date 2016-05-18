#include "Walker.hpp"

Walker::Walker(int d, int numSteps, UniformRNG &rng, hull_algorithm_t hull_algo)
    : numSteps(numSteps),
      d(d),
      rng(rng),
      hull_algo(hull_algo)
{
}

/** Get the number of random numbers used.
 */
int Walker::nRN() const
{
    return random_numbers.size();
}

void Walker::saveConfiguration(const std::string &filename, bool append)
{
    std::string data(serialize());

    auto mode = append ? std::ofstream::binary | std::ofstream::app : std::ofstream::binary;
    std::ofstream oss(filename, mode);
    if(!oss.good())
    {
        LOG(LOG_ERROR) << "File can not be opened: " << filename;
        return;
    }

    binary_write(oss, data.size());
    binary_write_string(oss, data);

    // save checksum?
    LOG(LOG_DEBUG) << "Save file   : " << filename;
}

std::string Walker::serialize()
{
    std::stringstream ss;

    // write header data: nsteps, d, number of random numbers, state of rng
    binary_write(ss, numSteps);
    binary_write(ss, d);
    //~ std::string rng_state(rng.serialize_rng());
    //~ binary_write(ss, rng_state.size());
    //~ binary_write_string(ss, rng_state);
    binary_write(ss, random_numbers.size());
    for(const double i : random_numbers)
        binary_write(ss, i);

    return ss.str();
}
