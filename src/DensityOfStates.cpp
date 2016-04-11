#include "DensityOfStates.hpp"

DensityOfStates::DensityOfStates(int bins, double lower, double upper)
    : bins(bins),
      lower(lower),
      upper(upper),
      data(std::vector<double>(bins, 1))
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

    int idx = (value - lower) / upper * bins;
    return data[idx];
}

void DensityOfStates::multiply(double value, double factor)
{
    if(!checkBounds(value))
        return;

    int idx = (value - lower) / upper * bins;
    data[idx] = data[idx] * factor;
}
