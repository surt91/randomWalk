#include "Walker.hpp"

void Walker::setHullAlgo(hull_algorithm_t a)
{
    hullDirty = true;
    hull_algo = a;
}

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

void Walker::loadConfiguration(const std::string &filename, int index)
{
    std::ifstream iss(filename, std::ifstream::binary);
    if(!iss.good())
    {
        LOG(LOG_ERROR) << "File can not be opened: " << filename;
        return;
    }

    size_t len = 0;
    int ctr = 0;
    while(ctr < index)
    {
        binary_read(iss, len);
        iss.seekg(len, iss.cur);
        ++ctr;
    }

    binary_read(iss, len);
    std::string data(binary_read_string(iss, len));

    // verify checksum?
    LOG(LOG_INFO) << "Read file   : " << filename;
    LOG(LOG_INFO) << "nsteps      : " << numSteps;
    LOG(LOG_INFO) << "dimension   : " << d;
    LOG(LOG_INFO) << "# random Num: " << random_numbers.size();
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

void Walker::deserialize(std::string s)
{
    size_t rn_size;
    std::stringstream ss;
    ss << s;

    binary_read(ss, numSteps);
    binary_read(ss, d);
    //~ size_t len_rng_state;
    //~ binary_read(iss, len_rng_state);
    //~ std::string rng_state(binary_read_string(iss, len_rng_state));
    //~ rng.deserialize_rng(rng_state);
    binary_read(ss, rn_size);

    random_numbers.clear();
    for(size_t i=0; i<rn_size; ++i)
    {
        double rn;
        binary_read(ss, rn);
        random_numbers.push_back(rn);
    }
}
