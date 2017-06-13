#include "SpecWalker.hpp"

/// Set the random numbers such that we get an L shape.
template <>
void SpecWalker<int>::degenerateMaxVolume()
{
    for(int i=0; i<numSteps; ++i)
        random_numbers[i] = .99 / ceil((double) d * (i+1)/numSteps);

    updateSteps();
    updatePoints();
    updateHull();
}

/// Set the random numbers such that we get an L shape in d-1 dimensions.
template <>
void SpecWalker<int>::degenerateMaxSurface()
{
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = .99 / ceil((double) (d-1) * (i+1)/random_numbers.size());

    updateSteps();
    updatePoints();
    updateHull();
}

/// Set the random numbers such that we get an one dimensional line.
template <>
void SpecWalker<int>::degenerateMinVolume()
{
    for(int i=0; i<numSteps; ++i)
        random_numbers[i] = .99;

    updateSteps();
    updatePoints();
    updateHull();
}

/// Set the random numbers such that we always step left, right, left, right.
template <>
void SpecWalker<int>::degenerateMinSurface()
{
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = i % 2 ? .99 : .99 - 1./d;

    updateSteps();
    updatePoints();
    updateHull();
}

/// Set the random numbers such that we get an half circle shape.
template <>
void SpecWalker<double>::degenerateMaxVolume()
{
    // FIXME: this works only for d=2
    if(d>2)
    {
        LOG(LOG_WARNING) << "Max Volume configuration is not really max volume";
    }
    for(int i=0; i<numSteps; ++i)
        random_numbers[i] = .5 / (i+1);

    updateSteps();
    updatePoints();
    updateHull();

    goDownhill(true, WO_VOLUME);
}

/// Set the random numbers such that we get an half circle shape in d-1 dimensions.
template <>
void SpecWalker<double>::degenerateMaxSurface()
{
    // FIXME: this works only for d<=3
    if(d>3)
    {
        LOG(LOG_WARNING) << "Max Surface configuration is not really max surface";
    }
    if(d==3)
        for(size_t i=0; i<random_numbers.size(); ++i)
            random_numbers[i] = .99 / ceil((double) (d-1) * (i+1)/random_numbers.size());
    if(d==2)
        for(size_t i=0; i<random_numbers.size(); ++i)
            random_numbers[i] = .99;

    updateSteps();
    updatePoints();
    updateHull();

    goDownhill(true, WO_SURFACE_AREA);
}

/// Set the random numbers such that we get an half circle shape in d-1 dimensions.
template <>
void SpecWalker<double>::degenerateMinVolume()
{
    // FIXME: this works only for d=2
    for(int i=0; i<numSteps; ++i)
        random_numbers[i] = .5 / (i+1);

    updateSteps();
    updatePoints();
    updateHull();

    goDownhill(false, WO_VOLUME);
}

/// Set the random numbers such that we get an L shape in d-1 dimensions.
template <>
void SpecWalker<double>::degenerateMinSurface()
{
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = .99 / ceil((double) (d-1) * (i+1)/random_numbers.size());

    updateSteps();
    updatePoints();
    updateHull();

    goDownhill(false, WO_SURFACE_AREA);
}
