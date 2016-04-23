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
    struct hash<Step<double>>;
}

/** Step class template, offers vector functions.
 */
template<class T>
class Step
{
    friend struct std::hash<Step<T>>;

    public:
        Step(){};
        Step(int /*d*/, double /*rn*/){throw std::invalid_argument("Step(int d, double rn) only implemented for Step<int>");};
        Step(int d)
            : m_d(d),
              m_coordinates(d, 0)
        {};
        Step(const std::vector<T> &coord)
            : m_d(coord.size()),
              m_coordinates(coord)
        {};

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

        // getter
        int d() const { return m_d; };
        T x(const int n=0) const { return m_coordinates[n]; };
        T y() const { return m_coordinates[1]; }; // specialisation for 2d
        T z() const { return m_coordinates[2]; }; // specialisation for 3d

        const std::vector<T>& coordinates() const { return m_coordinates; };

    protected:
        int m_d;
        std::vector<T> m_coordinates;

    private:

};

/** Specialization for int steps
 */
template <>
inline Step<int>::Step(int d, double rn)
      : m_d(d)
{
    m_coordinates = std::vector<int>(d, 0);
    for(int i=0; i<d; ++i)
        if(rn * d < i+1) // direction i
        {
            if(rn * d - i < 0.5)
                m_coordinates[i] = 1;
            else
                m_coordinates[i] = -1;

            break;
        }
}

template <class T>
double Step<T>::length() const
{
    double s=0;
    for(const auto i : m_coordinates)
        s += i*i;

    return std::sqrt(s);
}

/** only 2D projection on axis i and j
 */
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

    std::vector<T> new_coord = other.coordinates();
    for(int i=0; i<m_d; ++i)
        new_coord[i] += m_coordinates[i];

    return Step<T>(new_coord);
}

template <class T>
Step<T> Step<T>::operator-(const Step<T> &other) const
{
    if(m_d != other.d())
        throw std::invalid_argument("dimensions do not agree");

    std::vector<T> new_coord = other.coordinates();
    for(int i=0; i<m_d; ++i)
        new_coord[i] -= m_coordinates[i];

    return Step<T>(new_coord);
}

template <class T>
Step<T> Step<T>::operator-() const
{
    std::vector<T> new_coord = coordinates();
    for(int i=0; i<m_d; ++i)
        new_coord[i] *= -1;

    return Step<T>(new_coord);
}

template <class T>
Step<T> Step<T>::operator/(const double d) const
{
    std::vector<T> new_coord = coordinates();
    for(int i=0; i<m_d; ++i)
        new_coord[i] /= d;

    return Step<T>(std::move(new_coord));
}

template <class T>
Step<T>& Step<T>::operator+=(const Step<T> &other)
{
    if(m_d != other.d())
        throw std::invalid_argument("dimensions do not agree");

    const std::vector<T> other_coord = other.coordinates();
    for(int i=0; i<m_d; ++i)
        m_coordinates[i] += other_coord[i];

    return *this;
}

template <class T>
Step<T>& Step<T>::operator-=(const Step<T> &other)
{
    if(m_d != other.d())
        throw std::invalid_argument("dimensions do not agree");

    const std::vector<T> other_coord = other.coordinates();
    for(int i=0; i<m_d; ++i)
        m_coordinates[i] -= other_coord[i];

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

    std::vector<T> ret(3);
    ret[0] = a.y()*b.z() - a.z()*b.y();
    ret[1] = a.z()*b.x() - a.x()*b.z();
    ret[2] = a.x()*b.y() - a.y()*b.x();
    return Step<T>(ret);
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

template <class T>
void Step<T>::setZero()
{
    for(int i=0; i<m_d; ++i)
        m_coordinates[i] = T(0);
}

template <class T>
std::ostream& operator<<(std::ostream& os, const Step<T> &obj)
{
    os << "(";
    for(auto i : obj.m_coordinates)
        os << i << ", ";
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
                for(auto &i : s.m_coordinates)
                    seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
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
                for(auto &i : s.m_coordinates)
                    seed ^= std::hash<double>()(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
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
}
