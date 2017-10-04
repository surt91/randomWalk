#ifndef STEP_H
#define STEP_H

#include <cmath>
#include <list>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cassert>

template<class T>
class Step;
namespace std {
    template<>
    struct hash<Step<int>>;
    template<>
    struct hash<Step<double>>;
}

/** Step class template, offers vector functions.
 *
 * \tparam T datatype for the coordinates of the steps.
 *           int for steps on a lattice, double for real valued steps
 */
template<class T>
class Step
{
    protected:
        int m_d;
        #if D_MAX == 0
            std::vector<T> m_coordinates;
        #else
            std::array<T, D_MAX> m_coordinates;
        #endif

    public:
        /// Default constructor, init an d=0 empty step
        Step(){}

        /// Construct a d dimensional random Step from a random number.
        Step(int /*d*/, double /*rn*/){ throw std::invalid_argument("Step(int d, double rn) only implemented for Step<int>"); }
        /// Construct a d dimensional zero Step.
        explicit Step(int d);
        /// Construct a Step from a coordinate vector.
        explicit Step(const std::vector<T> &coord);
        explicit Step(const std::initializer_list<T> &coord);

        void fillFromRN(double /*rn*/, bool /*clean*/=false){ throw std::invalid_argument("fillFromRN(double rn, bool clean) only implemented for Step<int>"); }
        double readToRN(){ throw std::invalid_argument("readToRN() only implemented for Step<int>"); }
        std::vector<Step<int>> neighbors(bool /*diagonal*/=false) const { throw std::invalid_argument("neighbors() only implemented for Step<int>"); }
        std::vector<Step<int>> front_nneighbors(const Step<int> & /*direction*/) const { throw std::invalid_argument("front_nneighbors() only implemented for Step<int>"); }
        bool left_of(const Step<T> & /*direction*/) const { throw std::invalid_argument("left_of() only implemented for Step<int>"); }
        bool right_of(const Step<T> & /*direction*/) const { throw std::invalid_argument("right_of() only implemented for Step<int>"); }
        Step<T> left_turn() const { throw std::invalid_argument("left_turn() only implemented for Step<int>"); }
        Step<T> right_turn() const { throw std::invalid_argument("right_turn() only implemented for Step<int>"); }
        int winding_angle(const Step<T> & /*next*/) const { throw std::invalid_argument("winding_angle() only implemented for Step<int>"); }

        // properties
        double length() const;
        double length2() const;
        double angle(int i=0, int j=1) const;

        // comparison operators
        template <class U>
        friend inline bool operator==(const Step<U> &lhs, const Step<U> &rhs);

        template <class U>
        friend inline bool operator!=(const Step<U> &lhs, const Step<U> &rhs);

        bool operator<(const Step<T> &other) const;

        // arithmetic operators
        Step<T> operator+(const Step<T> &other) const;
        Step<T> operator-(const Step<T> &other) const;
        Step<T> operator-() const;
        Step<T> operator/(const double d) const;
        Step<T>& operator+=(const Step<T> &other);
        Step<T>& operator-=(const Step<T> &other);
        Step<T>& operator/=(const double d);

        T dist(const Step<T> &other) const;

        void periodic(const int len);
        void invert();

        template <class U>
        friend Step<U> cross(const Step<U> &a, const Step<U> &b);

        template <class U>
        friend double dot(const Step<U> &a, const Step<U> &b);

        void setZero();

        // output operators
        template <class U>
        friend std::ostream& operator<<(std::ostream& os, const Step<U> &obj);

        template <class U>
        friend std::ostream& operator<<(std::ostream& os, const std::vector<Step<U>> &obj);

        template <class U>
        friend std::ostream& operator<<(std::ostream& os, const std::list<Step<U>> &obj);

        // access operators
        T operator[](std::size_t idx) const { return m_coordinates[idx]; }
        T& operator[](std::size_t idx) { return m_coordinates[idx]; }

        size_t size() { return m_d; }

        void swap(Step<T>&) noexcept;

        // getter
        int d() const { return m_d; }
        T x(const int n=0) const { return m_coordinates[n]; }
        T y() const { return m_coordinates[1]; } // specialisation for 2d
        T z() const { return m_coordinates[2]; } // specialisation for 3d

        T max_coordinate() const { return std::max(m_coordinates.begin(), m_coordinates.end()); }

    private:
        friend struct std::hash<Step<T>>;
};

/// Fills the Step according to rn. Assumes Step is 0 if clean is true
template <>
inline void Step<int>::fillFromRN(double rn, bool clean)
{
    if(!clean)
        setZero();

    // TODO: replace loop by direct calculation
    for(int i=0; i<m_d; ++i)
        if(rn * m_d < i+1) // direction i
        {
            if(rn * m_d - i < 0.5)
                m_coordinates[i] = 1;
            else
                m_coordinates[i] = -1;

            // this break is important, otherwise all dimensions
            // after the wanted one will be filled by -1
            break;
        }
}

/// returns a "random number" which will fill a Step with the same values as
/// this, if used with fillFromRN()
template <>
inline double Step<int>::readToRN()
{
    for(int i=0; i<m_d; ++i)
        if(m_coordinates[i] != 0)
        {
            double rn = i / (double) m_d + 0.5 / (2*m_d);
            if(m_coordinates[i] == -1)
                rn += 0.5 / m_d;

            return rn;
        }
    return 0;
}

/// Yields neighbors
template <>
inline std::vector<Step<int>> Step<int>::neighbors(bool diagonal) const
{
    // A d dimensional hypercube has 2*d direct neighbors, in positive
    // and negative direction for every dimension.
    int num_neighbors = 2*m_d;
    if(diagonal)
    {
        if(m_d == 2)
            num_neighbors += 4;
        if(m_d == 3)
            num_neighbors += 8 + 12;
    }

    std::vector<Step<int>> ret(num_neighbors, *this);
    for(int i=0; i<m_d; ++i)
    {
        ret[2*i][i] += 1;
        ret[2*i+1][i] -= 1;
    }

    if(diagonal)
    {
        if(m_d > 3)
            throw std::invalid_argument("not implemented for d > 3");

        Step<int> diff(m_d);
        // A d dimensional hypercube has 2^d diagonal elements
        // we can enumerate them with bitfiddling, i.e., to
        // generate all 2^d possibilities, we take all numbers
        // up to 2^d-1 and interpret their j-th bit as a displacement
        // in negative direction for 0 and positive for 1.
        // The sum of the center and this displacement generates
        // all cornerstones.
        for(int i=0; i<std::pow(2, m_d); ++i)
        {
            for(int j=0; j<m_d; ++j)
            {
                diff[j] = (i & (1 << j)) ? 1 : -1;
            }
            ret[2*m_d + i] += diff;
        }

        // in d = 3 there are also the sides
        // for each direction, leave it at zero and generate all combinations
        // of the remaining two directions
        if(m_d == 3)
        {
            for(int i=0; i<std::pow(2, m_d-1); ++i)
            {
                // wrong!
                for(int j=0; j<m_d; ++j)
                {
                    diff[j] = 0;
                    for(int k=0, l=0; k<m_d; ++k)
                    {
                        if(k != j)
                        {
                            diff[k] = (i & (1 << l)) ? 1 : -1;
                            ++l;
                        }
                    }
                    ret[2*m_d + i*std::pow(2, m_d-1) + j] += diff;
                }
            }
        }

        // I can not imagine d > 3 easily. There should be one category more
        // where two coordinates are the same and two change.
        // I will implement it as soon as I need it.
    }

    return ret;
}

/** Yields neighbors in front of a 2-dimensional step
 *
 * this should be a position
 *
 * \param direction should be a step of length 1 indication the direction
 * i.e. the step taken to arrive at the position
 *
 * \returns a vector of positions in front of this position
 */
template <>
inline std::vector<Step<int>> Step<int>::front_nneighbors(const Step<int> &direction) const
{
    // front_nneighbors() is only implemented for d=2
    assert(m_d == 2);

    // build a step that steps to the right side
    Step<int> orthogonal(m_d);
    if(direction.m_coordinates[0])
        orthogonal.m_coordinates[1] = -direction.m_coordinates[0];
    else
        orthogonal.m_coordinates[0] = direction.m_coordinates[1];

    std::vector<Step<int>> ret(5, *this);
    ret[0] += direction;  // straight ahead
    ret[1] -= orthogonal; // left
    ret[2] += orthogonal; // right
    ret[3] += direction;
    ret[3] -= orthogonal; // front left
    ret[4] += direction;
    ret[4] += orthogonal; // front right

    return ret;
}

/// is step this a left turn from the point of view of `direction`
template <>
inline bool Step<int>::left_of(const Step<int> &direction) const
{
    // left_of() is only implemented for d=2!
    assert(m_d == 2);

    if(m_coordinates[0])
    {
        if(m_coordinates[0] == -direction.m_coordinates[1])
            return true;
    }
    else
    {
        if(m_coordinates[1] == direction.m_coordinates[0])
            return true;
    }

    return false;
}

/// is step this a right turn from the point of view of `direction`
template <>
inline bool Step<int>::right_of(const Step<int> &direction) const
{
    // right_of() is only implemented for d=2!
    assert(m_d == 2);

    if(m_coordinates[0])
    {
        if(m_coordinates[0] == direction.m_coordinates[1])
            return true;
    }
    else
    {
        if(m_coordinates[1] == -direction.m_coordinates[0])
            return true;
    }

    return false;
}

/// return step which is a left turn from the point of view of `direction`
template <>
inline Step<int> Step<int>::left_turn() const
{
    // left_turn() is only implemented for d=2!
    assert(m_d == 2);

    Step<int> ret(m_d);

    if(m_coordinates[0])
        ret[1] = m_coordinates[0];
    else
        ret[0] = -m_coordinates[1];

    return ret;
}

/// retrun step which is a right turn from the point of view of `direction`
template <>
inline Step<int> Step<int>::right_turn() const
{
    // right_turn() is only implemented for d=2!
    assert(m_d == 2);

    Step<int> ret(m_d);

    if(m_coordinates[0])
        ret[1] = -m_coordinates[0];
    else
        ret[0] = m_coordinates[1];

    return ret;
}

/** winding angle between two steps
 *
 * the arguments need to be steps of length 1 and not positions
 *
 * only well defined in d=2 for neighboring steps on a square lattice
 *  +1 for right turn
 *  -1 for left turn
 *   0 for straight
 */
template <>
inline int Step<int>::winding_angle(const Step<int> &next) const
{
    if(next.left_of(*this))
        return -1;
    if(next.right_of(*this))
        return 1;
    return 0;
    //
    // throw std::invalid_argument("you use the winding_angle function wrong!");
}

#if D_MAX == 0
/// Construct a d dimensional zero Step.
template <class T>
Step<T>::Step(int d)
    : m_d(d),
      m_coordinates(d, 0)
{
}
#else
/// Construct a d dimensional zero Step.
template <class T>
Step<T>::Step(int d)
    : m_d(d)
{
    for(int i=0; i<m_d; ++i)
        m_coordinates[i] = 0;
}
#endif

#if D_MAX == 0
/// Construct a Step from a coordinate vector.
template <class T>
Step<T>::Step(const std::vector<T> &coord)
    : m_d(coord.size()),
      m_coordinates(coord)
{

}
template <class T>
Step<T>::Step(const std::initializer_list<T> &coord)
    : m_d(coord.size()),
      m_coordinates(begin(coord), end(coord))
{

}
#else
/// Construct a Step from a coordinate vector.
template <class T>
Step<T>::Step(const std::vector<T> &coord)
    : m_d(coord.size())
{
    for(int i=0; i<m_d; ++i)
        m_coordinates[i] = coord[i];
}
template <class T>
Step<T>::Step(const std::initializer_list<T> &coord)
    : m_d(coord.size())
{
    int j = 0;
    for(auto i=begin(coord); i<begin(coord)+m_d; ++i)
        m_coordinates[j++] = *i;
}
#endif

/// Specialization for int steps.
template <>
inline Step<int>::Step(int d, double rn)
      : m_d(d)
#if D_MAX == 0
        , m_coordinates(d, 0)
#endif
{
    fillFromRN(rn);
}

/// Euclidean distance to zero.
template <class T>
double Step<T>::length() const
{
    return std::sqrt(length2());
}

/// Squared Euclidean distance to zero.
template <class T>
double Step<T>::length2() const
{
    double s=0;
    for(int i=0; i<m_d; ++i)
        s += m_coordinates[i]*m_coordinates[i];

    return s;
}

/// Only 2D projection on axis i and j.
template <class T>
double Step<T>::angle(int i, int j) const
{
    return atan2(m_coordinates[j], m_coordinates[i]);
}

template <class T>
bool operator==(const Step<T> &lhs, const Step<T> &rhs)
{
    if(lhs.m_d != rhs.m_d)
        return false;

    for(int i=0; i<lhs.m_d; ++i)
        if(lhs.m_coordinates[i] != rhs.m_coordinates[i])
            return false;

    return true;
}

template <class T>
bool operator!=(const Step<T> &lhs, const Step<T> &rhs)
{
    return ! (lhs == rhs);
}

template <class T>
Step<T> Step<T>::operator+(const Step<T> &other) const
{
    // dimensions do not agree
    assert(m_d == other.d());

    Step<T> new_step(other);
    for(int i=0; i<m_d; ++i)
        new_step[i] += m_coordinates[i];

    return new_step;
}

template <class T>
Step<T> Step<T>::operator-(const Step<T> &other) const
{
    // dimensions do not agree
    assert(m_d == other.d());

    Step<T> new_step(*this);
    for(int i=0; i<m_d; ++i)
        new_step[i] -= other.m_coordinates[i];

    return new_step;
}

template <class T>
Step<T> Step<T>::operator-() const
{
    Step<T> new_step(*this);
    for(int i=0; i<m_d; ++i)
        new_step[i] *= -1;

    return new_step;
}

template <class T>
Step<T> Step<T>::operator/(const double d) const
{
    std::vector<T> new_coord = m_coordinates;
    for(int i=0; i<m_d; ++i)
        new_coord[i] /= d;

    return Step<T>(std::move(new_coord));
}

template <class T>
Step<T>& Step<T>::operator+=(const Step<T> &other)
{
    // dimensions do not agree
    assert(m_d == other.m_d);

    for(int i=0; i<m_d; ++i)
        m_coordinates[i] += other.m_coordinates[i];

    return *this;
}

template <class T>
Step<T>& Step<T>::operator-=(const Step<T> &other)
{
    // dimensions do not agree
    assert(m_d == other.m_d);

    for(int i=0; i<m_d; ++i)
        m_coordinates[i] -= other.m_coordinates[i];

    return *this;
}

template <class T>
Step<T>& Step<T>::operator/=(const double d)
{
    // uncool, since my coordinates can be integer
    for(int i=0; i<m_d; ++i)
        m_coordinates[i] = std::round(m_coordinates[i]/d);

    return *this;
}

/** a point is smaller, if the x value is smaller
 * in case of a tie the smaller y value decides
 * we need to do in in arbitrary dimensions to make std::set of Step possible
 */
template <class T>
bool Step<T>::operator<(const Step<T> &other) const
{
    for(int i=0; i<m_d; ++i)
    {
        if(x(i) == other.x(i))
            continue;
        return x(i) < other.x(i);
    }
    // apperently all coordinates are equal
    return 0;
}

template <class T>
Step<T> cross(const Step<T> &a, const Step<T> &b)
{
    // dimensions do not agree
    assert(a.m_d == b.m_d);
    // cross product only defined for d=3
    assert(a.m_d == 3);

    Step<T> ret(3);
    ret[0] = a.y()*b.z() - a.z()*b.y();
    ret[1] = a.z()*b.x() - a.x()*b.z();
    ret[2] = a.x()*b.y() - a.y()*b.x();
    return ret;
}

template <class T>
double dot(const Step<T> &a, const Step<T> &b)
{
    // dimensions do not agree
    assert(a.m_d == b.m_d);

    double p = 0;

    for(int i=0; i<a.m_d; ++i)
        p += a[i]*b[i];

    return p;
}

/// Manhattan distance
template<>
inline int Step<int>::dist(const Step<int> &other) const
{
    // dimensions do not agree
    assert(m_d == other.m_d);

    int d = 0;
    for(int i=0; i<m_d; ++i)
        d += std::abs(x(i) - other.x(i));

    return d;
}

/// Euclidean distance
template<>
inline double Step<double>::dist(const Step<double> &other) const
{
    // dimensions do not agree
    assert(m_d == other.m_d);

    double d = 0;

    for(int i=0; i<m_d; ++i)
    {
        double t = x(i) - other.x(i);
        d += t*t;
    }

    d = std::sqrt(d);

    return d;
}

/// ensures that the step is inside [0, len]^d with periodic boundaries
template <class T>
void Step<T>::periodic(const int len)
{
    // FIXME will not always work
    for(auto &k : m_coordinates)
        k = (k+len) % len;
}

/// inverts the step
template <class T>
void Step<T>::invert()
{
    for(auto &k : m_coordinates)
        k *= -1;
}

template <class T>
void Step<T>::setZero()
{
    std::fill(begin(m_coordinates), begin(m_coordinates)+m_d, 0);
}

template <class T>
std::ostream& operator<<(std::ostream& os, const Step<T> &obj)
{
    os << "(";
    for(int i=0; i<obj.m_d; ++i)
        os << obj.m_coordinates[i] << ", ";
    os << ") ";
    return os;
}

// will write any std::container of Steps to a stream
template <
    class T,
    template <class, class...> class Container,
    class... AddParams
>
std::ostream& operator<<(std::ostream& os, const Container<Step<T>, AddParams...> &obj)
{
    os << "[";
    for(const auto &i : obj)
        os << i << " ";
    os << "]";
    return os;
}

template <class T>
void Step<T>::swap(Step<T>& other) noexcept
{
    int tmp = m_d;
    m_d = other.m_d;
    other.m_d = tmp;

    m_coordinates.swap(other.m_coordinates);
}

template<class T>
void swap(Step<T>& lhs, Step<T>& rhs) { lhs.swap(rhs); }

namespace std {
    template<>
    struct hash<Step<int>>
    {
        public:
            /* very simple hashing for int vectors
             *
             * http://stackoverflow.com/a/27216842/1698412
             */
            size_t operator()(const Step<int> &s) const
            {
                std::size_t seed = 0;
                for(int i=0; i<s.m_d; ++i)
                    seed ^= s.m_coordinates[i] + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                return seed;
            }
    };

    template<>
    struct hash<Step<double>>
    {
        public:
            size_t operator()(const Step<double> &s) const
            {
                std::size_t seed = 0;
                for(int i=0; i<s.m_d; ++i)
                    seed ^= std::hash<double>()(s.m_coordinates[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                return seed;
            }
    };

    template <class T>
    Step<T> min(const std::vector<Step<T>> &v)
    {
        Step<T> p = v[0];
        for(const auto &i : v)
            if(i < p)
                p = i;
        return p;
    }

    template <class T>
    void swap(Step<T>& lhs, Step<T>& rhs) { lhs.swap(rhs); }
}

#endif
