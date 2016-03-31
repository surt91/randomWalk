#include "ConvexHull.hpp"


ConvexHull::ConvexHull(const std::vector<Step>& interiorPoints, bool /*akl*/)
            : interiorPoints(interiorPoints),
              n(interiorPoints.size()),
              d(interiorPoints[0].d())
{
}

ConvexHull::~ConvexHull()
{
}

// deletes points from interior points according to the Akl Toussaint heuristic
void ConvexHull::preprocessAklToussaint()
{
    throw std::invalid_argument("Akl Toussaint Heuristic currently only implemented for d=2");
}

std::vector<std::vector<Step>> ConvexHull::hullFacets() const
{
    throw std::invalid_argument("hull facets for this algorithm not implemented");
}
