#include "DensityOfStates.hpp"

DensityOfStates::DensityOfStates(int bins, double lower, double upper)
    : bins(bins),
      lower(lower),
      upper(upper),
      binwidth((upper - lower) / bins),
      data(bins, 0)
{
    fail = 1.0;
}

bool DensityOfStates::checkBounds(double value)
{
    if(value > upper || value < lower)
    {
        LOG(LOG_ERROR) << "value out of bounds: " << value << " not in [" << lower << "," << upper << "]";
        return false;
    }
    return true;
}

double& DensityOfStates::operator[](double value)
{
    if(!checkBounds(value))
        return fail;

    int idx = (value - lower) / binwidth;
    return data[idx];
}

void DensityOfStates::reset()
{
    for(int i=0; i<bins; ++i)
        data[i] = 0;
}

std::string DensityOfStates::binCentersString()
{
    std::stringstream out;
    out.precision(12);
    for(int i=0; i<bins; ++i)
        out << ((i+0.5)*binwidth+lower) << " ";
    return out.str();
}

std::string DensityOfStates::dataString()
{
    std::stringstream out;
    out.precision(12);
    for(const auto i : data)
        out << i << " ";
    return out.str();
}

std::ostream& operator<<(std::ostream& os, const DensityOfStates &obj)
{
    os << "[";
    for(int i=0; i<obj.bins; ++i)
        os << (i*obj.binwidth+obj.lower) << " " << obj.data[i] << std::endl;
    os << "] ";
    return os;
}

DensityOfStates& DensityOfStates::operator+=(const DensityOfStates &other)
{
    for(int i=0; i<bins; ++i)
        data[i] += other.data[i];

    return *this;
}
