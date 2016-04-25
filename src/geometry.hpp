#pragma once

#include "Step.hpp"

/** 2D cross product of OA and OB vectors, i.e. z-component of their 3D cross product.
 * Returns a positive value, if OAB makes a counter-clockwise turn,
 * negative for clockwise turn, and zero if the points are collinear.
 */
template <class T>
T cross2d_z(const Step<T>& O, const Step<T>& A, const Step<T>& B)
{
    return (A.x() - O.x()) * (B.y() - O.y()) - (A.y() - O.y()) * (B.x() - O.x());
}

/** Test if point p is inside the triangel formed by p1, p2, p3.
 *
 * Sequence of the points matters, must be counterclockwise.
 */
template <class T>
bool pointInTriangle(const Step<T>& p1, const Step<T>& p2, const Step<T>& p3, const Step<T>& p)
{
    bool checkSide1 = side(p1, p2, p) >= 0;
    bool checkSide2 = side(p2, p3, p) >= 0;
    bool checkSide3 = side(p3, p1, p) >= 0;
    return checkSide1 && checkSide2 && checkSide3;
}

/** Test if point p is inside the quadriliteral formed by p1, p2, p3, p4.
 *
 * Sequence of the points matters, must be counterclockwise.
 */
template <class T>
bool pointInQuadrilateral(const Step<T>& p1, const Step<T>& p2, const Step<T>& p3, const Step<T>& p4, const Step<T>& p)
{
    bool checkSide1 = side(p1, p2, p) >= 0;
    bool checkSide2 = side(p2, p3, p) >= 0;
    bool checkSide3 = side(p3, p4, p) >= 0;
    bool checkSide4 = side(p4, p1, p) >= 0;
    return checkSide1 && checkSide2 && checkSide3 && checkSide4;
}

/** Test if point p is inside the polygon formed by the points in poly.
 *
 * Sequence of the points matters, must be counterclockwise.
 */
template <class T>
bool pointInPolygon(const std::vector<Step<T>>& poly, const Step<T>& p)
{
    bool inside = side(poly[poly.size()-1], poly[0], p) >= 0;
    for(size_t i=1; i<poly.size(); ++i)
        inside = inside && (side(poly[i-1], poly[i], p) >= 0);

    return inside;
}

/** Test on which side of the line p1, p2 the point p lies.
 *
 * 1 for left, -1 for right (or the other way around), 0 for on the line.
 *
 * This implementation does only work for d=2.
 */
template <class T>
T side(const Step<T>& p1, const Step<T>& p2, const Step<T>& p)
{
    return (p2.y() - p1.y())*(p.x() - p1.x()) + (-p2.x() + p1.x())*(p.y() - p1.y());
}
