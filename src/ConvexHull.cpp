#include "ConvexHull.hpp"


ConvexHull::ConvexHull(const std::vector<Step>& interiorPoints)
            : interiorPoints(interiorPoints),
              n(interiorPoints.size()),
              d(interiorPoints[0].d())
{
}

ConvexHull::~ConvexHull()
{
}
