#pragma once

#include <cmath>
#include <list>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <stdexcept>
#include <algorithm>

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
        Step(){};

        /// Construct a d dimensional random Step from a random number.
        Step(int /*d*/, double /*rn*/){ throw std::invalid_argument("Step(int d, double rn) only implemented for Step<int>"); };
        /// Construct a d dimensional zero Step.
        explicit Step(int d);
        /// Construct a Step from a coordinate vector.
        explicit Step(const std::vector<T> &coord);

        void fillFromRN(double /*rn*/, bool clean=false){ throw std::invalid_argument("fillFromRN(double rn, bool clean) only implemented for Step<int>"); };
        std::vector<Step<int>> neighbors(bool diagonal=false) const { throw std::invalid_argument("neighbors() only implemented for Step<int>"); };

        // properties
        double length() const;
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

        int dist(const Step<int> &other) const;
        double dist(const Step<double> &other) const;

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
        T operator[](std::size_t idx) const { return m_coordinates[idx]; };
        T& operator[](std::size_t idx) { return m_coordinates[idx]; };

        void swap(Step<T>&) throw();

        // getter
        int d() const { return m_d; };
        T x(const int n=0) const { return m_coordinates[n]; };
        T y() const { return m_coordinates[1]; }; // specialisation for 2d
        T z() const { return m_coordinates[2]; }; // specialisation for 3d

        T max_coordinate() const { return std::max(m_coordinates.begin(), m_coordinates.end()); };

    private:
        friend struct std::hash<Step<T>>;
};

/// Fills the Step according to rn. Assumes Step is 0 if clean is true
template <>
inline void Step<int>::fillFromRN(double rn, bool clean)
{
    if(!clean)
        setZero();
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

/// Yields neighbors
template <>
inline std::vector<Step<int>> Step<int>::neighbors(bool diagonal) const
{
    // A d dimensional hypercube has 2*d direct neighbors, in positive
    // and negative direction for every dimension.
    std::vector<Step<int>> ret(2*m_d + std::pow(2, m_d), *this);
    for(int i=0; i<m_d; ++i)
    {
        ret[2*i][i] += 1;
        ret[2*i+1][i] -= 1;
    }

    if(diagonal)
    {
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
    }
    else
    {
        ret.resize(2*m_d);
    }

    return ret;
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
#else
/// Construct a Step from a coordinate vector.
template <class T>
Step<T>::Step(const std::vector<T> &coord)
    : m_d(coord.size())
{
    for(int i=0; i<m_d; ++i)
        m_coordinates[i] = coord[i];
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
    double s=0;
    for(int i=0; i<m_d; ++i)
        s += m_coordinates[i]*m_coordinates[i];

    return std::sqrt(s);
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
    if(m_d != other.d())
        throw std::invalid_argument("dimensions do not agree");

    Step<T> new_step(other);
    for(int i=0; i<m_d; ++i)
        new_step[i] += m_coordinates[i];

    return new_step;
}

template <class T>
Step<T> Step<T>::operator-(const Step<T> &other) const
{
    if(m_d != other.d())
        throw std::invalid_argument("dimensions do not agree");

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
    // TODO: replace by assert
    if(m_d != other.m_d)
        throw std::invalid_argument("dimensions do not agree");

    for(int i=0; i<m_d; ++i)
        m_coordinates[i] += other.m_coordinates[i];

    return *this;
}

template <class T>
Step<T>& Step<T>::operator-=(const Step<T> &other)
{
    if(m_d != other.d())
        throw std::invalid_argument("dimensions do not agree");

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
 * only for 2d, since it is only needed for the 2d andrew algorithm
 */
template <class T>
bool Step<T>::operator<(const Step<T> &other) const
{
    return x() < other.x() || (x() == other.x() && y() < other.y());
}

template <class T>
Step<T> cross(const Step<T> &a, const Step<T> &b)
{
    if(a.m_d != b.m_d)
        throw std::invalid_argument("dimensions do not agree");
    if(a.m_d != 3)
        throw std::invalid_argument("cross product only defined for d=3");

    Step<T> ret(3);
    ret[0] = a.y()*b.z() - a.z()*b.y();
    ret[1] = a.z()*b.x() - a.x()*b.z();
    ret[2] = a.x()*b.y() - a.y()*b.x();
    return ret;
}

template <class T>
double dot(const Step<T> &a, const Step<T> &b)
{
    if(a.m_d != b.m_d)
        throw std::invalid_argument("dimensions do not agree");

    double p = 0;

    for(int i=0; i<a.m_d; ++i)
        p += a[i]*b[i];

    return p;
}

/// Manhattan distance
template<>
inline int Step<int>::dist(const Step<int> &other) const
{
    if(m_d != other.m_d)
        throw std::invalid_argument("dimensions do not agree");

    int d = 0;
    for(int i=0; i<m_d; ++i)
        d += std::abs(x(i) - other.x(i));

    return d;
}

/// Euclidean distance
template<>
inline double Step<double>::dist(const Step<double> &other) const
{
    if(m_d != other.m_d)
        throw std::invalid_argument("dimensions do not agree");

    double d = 0;

    for(int i=0; i<m_d; ++i)
    {
        double t = x(i) - other.x(i);
        d += t*t;
    }

    d = std::sqrt(d);

    return d;
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

template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<Step<T>> &obj)
{
    os << "[";
    for(const auto &i : obj)
        os << i << " ";
    os << "]";
    return os;
}

template <class T>
std::ostream& operator<<(std::ostream& os, const std::list<Step<T>> &obj)
{
    os << "[";
    for(const auto &i : obj)
        os << i << " ";
    os << "]";
    return os;
}

template <class T>
void Step<T>::swap(Step<T>& other) throw()
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
