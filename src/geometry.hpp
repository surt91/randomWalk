#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <array>

#include <Qhull.h>
#include <QhullVertex.h>
#include <QhullFacetList.h>
#include <QhullVertexSet.h>
#include <QhullError.h>

#include "Step.hpp"

/** 2D cross product of OA and OB vectors, i.e. z-component of their 3D cross product.
 * Returns a positive value, if OAB makes a counter-clockwise turn,
 * negative for clockwise turn, and zero if the points are collinear.
 */
template <class T>
T cross2d_z(const Step<T>& O,
            const Step<T>& A,
            const Step<T>& B)
{
    return (A.x() - O.x()) * (B.y() - O.y()) - (A.y() - O.y()) * (B.x() - O.x());
}

template <class T>
T cross2d_z(const std::array<T, 2>& O,
            const std::array<T, 2>& A,
            const std::array<T, 2>& B)
{
    return (A[0] - O[0]) * (B[1] - O[1]) - (A[1] - O[1]) * (B[0] - O[0]);
}

/** Test if point p is inside the triangel formed by p1, p2, p3.
 *
 * Sequence of the points matters, must be counterclockwise.
 */
template <class T>
bool pointInTriangle(const Step<T>& p1,
                     const Step<T>& p2,
                     const Step<T>& p3,
                     const Step<T>& p)
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
bool pointInQuadrilateral(const Step<T>& p1,
                          const Step<T>& p2,
                          const Step<T>& p3,
                          const Step<T>& p4,
                          const Step<T>& p)
{
    bool checkSide1 = side(p1, p2, p) >= 0;
    bool checkSide2 = side(p2, p3, p) >= 0;
    bool checkSide3 = side(p3, p4, p) >= 0;
    bool checkSide4 = side(p4, p1, p) >= 0;
    return checkSide1 && checkSide2 && checkSide3 && checkSide4;
}

/** Test if point p is inside the octoliteral formed by p1 to p8.
 *
 * Sequence of the points matters, must be counterclockwise.
 */
template <class T>
bool pointInOcto(const Step<T>& p1,
                 const Step<T>& p2,
                 const Step<T>& p3,
                 const Step<T>& p4,
                 const Step<T>& p5,
                 const Step<T>& p6,
                 const Step<T>& p7,
                 const Step<T>& p8,
                 const Step<T>& p)
{
    // compare opposite sites first
    return side(p1, p2, p) >= 0
        && side(p3, p4, p) >= 0
        && side(p5, p6, p) >= 0
        && side(p7, p8, p) >= 0
        && side(p2, p3, p) >= 0
        && side(p4, p5, p) >= 0
        && side(p6, p7, p) >= 0
        && side(p8, p1, p) >= 0;
}

/** Test if point p is inside the octahedron formed by the given vertices
 */
template <class T>
bool pointInOctahedron(const Step<T>& x,
                       const Step<T>& X,
                       const Step<T>& y,
                       const Step<T>& Y,
                       const Step<T>& z,
                       const Step<T>& Z,
                       const Step<T>& p)
{
    return side(X, Y, Z, p) >= 0
        && side(X, Z, y, p) >= 0
        && side(X, z, Y, p) >= 0
        && side(X, y, z, p) >= 0
        && side(x, Z, Y, p) >= 0
        && side(x, y, Z, p) >= 0
        && side(x, Y, z, p) >= 0
        && side(x, z, y, p) >= 0;
}

template <class T>
bool pointInFacets(orgQhull::QhullFacetList fl, const Step<T>& p)
{
    for(auto &f : fl)
        if(side(f, p) < 0)
            return false;
    return true;
}

/** Test if point p is inside the Quattuordecaeder (star tetraeder)
 * formed by the given vertices
 * https://upload.wikimedia.org/wikipedia/commons/1/19/Stella_octangula.png
 */
template <class T>
bool pointInQuattuordecaeder(const Step<T>& x,
                             const Step<T>& X,
                             const Step<T>& y,
                             const Step<T>& Y,
                             const Step<T>& z,
                             const Step<T>& Z,
                             const Step<T>& ppp,
                             const Step<T>& ppm,
                             const Step<T>& pmp,
                             const Step<T>& mpp,
                             const Step<T>& pmm,
                             const Step<T>& mpm,
                             const Step<T>& mmp,
                             const Step<T>& mmm,
                             const Step<T>& p)
{
    return side(x, mpm, mmm, p) >= 0
        && side(x, mmm, mmp, p) >= 0
        && side(x, mmp, mpp, p) >= 0
        && side(x, mpp, mpm, p) >= 0
        && side(y, pmp, mmp, p) >= 0
        && side(y, mmp, mmm, p) >= 0
        && side(y, mmm, pmm, p) >= 0
        && side(y, pmm, pmp, p) >= 0
        && side(z, mmm, mpm, p) >= 0
        && side(z, mpm, ppm, p) >= 0
        && side(z, ppm, pmm, p) >= 0
        && side(z, pmm, mmm, p) >= 0
        && side(X, ppp, pmp, p) >= 0
        && side(X, pmp, pmm, p) >= 0
        && side(X, pmm, ppm, p) >= 0
        && side(X, ppm, ppp, p) >= 0
        && side(Y, mpp, ppp, p) >= 0
        && side(Y, ppp, ppm, p) >= 0
        && side(Y, ppm, mpm, p) >= 0
        && side(Y, mpm, mpp, p) >= 0
        && side(Z, ppp, mpp, p) >= 0
        && side(Z, mpp, mmp, p) >= 0
        && side(Z, mmp, pmp, p) >= 0
        && side(Z, pmp, ppp, p) >= 0;
}

/** Test if point p is inside the Polygon formed by the N points in the array.
 *
 * Sequence of the points matters, must be counterclockwise.
 */
template <class T, size_t N>
bool pointInPoly(const std::array<Step<T>&, N>& poly, const Step<T>& p)
{
    bool inside = side(poly[N-1], poly[0], p) >= 0;
    for(size_t i=1; i<N; ++i)
        inside = inside && (side(poly[i-1], poly[i], p) >= 0);

    return inside;
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
T side(const Step<T>& p1,
       const Step<T>& p2,
       const Step<T>& p)
{
    return (p2.y() - p1.y())*(p.x() - p1.x()) + (-p2.x() + p1.x())*(p.y() - p1.y());
}

/** Test on which side of the plane p1, p2, p3 the point p lies.
 *
 * p1, p2, p3 must be ordered counter clockwise (seen from outside / the front).
 *
 * >0 for behind, <0 for in front, 0 for on the plane.
 *
 * This implementation does only work for d=3.
 */
template <class T>
T side(const Step<T>& p1,
       const Step<T>& p2,
       const Step<T>& p3,
       const Step<T>& p)
{
    // calculate dot product of
    //   1. vector from p to some point on the face
    //   2. normal vector (obtained by cross product)
    // if they have a parallel component -> behind -> dot > 0
    // if they have a anitparallel component -> infront -> dot < 0

    //~ return dot(p1-p, cross(p2-p1, p3-p1));

    // formulate it directly by hand, to avoid temporaries
    // totally premature optimization, but speeds it up somewhat
    auto c1x = p2.x() - p1.x();
    auto c2x = p3.x() - p1.x();
    auto c1y = p2.y() - p1.y();
    auto c2y = p3.y() - p1.y();
    auto c1z = p2.z() - p1.z();
    auto c2z = p3.z() - p1.z();
    return  (p1.x()-p.x()) * (c1y*c2z - c1z*c2y)
          + (p1.y()-p.y()) * (c1z*c2x - c1x*c2z)
          + (p1.z()-p.z()) * (c1x*c2y - c1y*c2x);
}

/** Test on which side of the qhull facet f the point p lies.
 *
 * >0 for behind, <0 for in front, 0 for on the plane.
 *
 * This implementation does only work for d=3.
 */
template <class T>
T side(orgQhull::QhullFacet& f, const Step<T>& p)
{
    // same principle as above
    auto v = (pointT*) f.getFacetT()->vertices->e[0].p;
    auto normal = f.getFacetT()->normal;

    auto s = (v[0]-p.x()) * normal[0]
           + (v[1]-p.y()) * normal[1]
           + (v[2]-p.z()) * normal[2];
    return s < 0 ? -1 : 1;
}

template <class T>
int orientation(Step<T> O, Step<T> A, Step<T> B)
{
	// T val = (A[1] - O[1]) * (B[0] - A[0]) - (A[0] - O[0]) * (B[1] - A[1]);
    T val = -cross2d_z(O, A, B);
	if (val == 0) return 0;  // Collinear
	return (val > 0)? -1: 1; // CW: -1 or CCW: 1
}

// cmp: https://github.com/ypranay/Convex-Hull/blob/master/ChansAlgorithmForConvexHull.cpp
template <class T>
Step<T> polygon_tangent(const Step<T> &p, const std::vector<Step<T>> &v)
{
    // search for the tangent through `p` of the polygon `poly`
    // use a clever binary search
    // all points q before the tangent t are ptq oriented ccw and after cw

    int n = v.size();
    int l = 0;
    int r = n;
    int l_before = orientation(p, v[0], v[n-1]);
    int l_after = orientation(p, v[0], v[(l + 1) % n]);
    while (l < r){
        int c = (l + r)/2;
        int c_before = orientation(p, v[c], v[(c - 1) % n]);
        int c_after = orientation(p, v[c], v[(c + 1) % n]);
        int c_side = orientation(p, v[l], v[c]);
        if (c_before >= 0 && c_after >= 0)
            return v[c];
        else if (((c_side > 0) && (l_after < 0 || l_before == l_after)) || (c_side < 0 && c_before < 0))
            r = c;
        else
            l = c + 1 ;
        l_before = -c_after;
        l_after = orientation(p, v[l%n], v[(l + 1) % n]);
    }
    return v[l%n];
}

#endif
