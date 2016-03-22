#pragma once

#include <cmath>
#include <vector>
#include <iostream>
#include <stdexcept>

class Step;
namespace std {
    template<>
    struct hash<Step>;
}

class Step
{
    friend struct std::hash<Step>;

    public:
        Step()
        {};

        Step(int d, double rn)
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
        };

        Step(const std::vector<int> &coord)
              : m_d(coord.size()),
                m_coordinates(coord)
              {};

        double angle(int i=0, int j=1) const //only 2D projection on axis i and j
        {
            return atan2(m_coordinates[j], m_coordinates[i]);
        }

        friend inline bool operator==(const Step &lhs, const Step &rhs);

        Step operator+(const Step &other) const
        {
            if(m_d != other.d())
                throw std::invalid_argument("dimensions do not agree");

            std::vector<int> new_coord = other.coordinates();
            for(int i=0; i<m_d; ++i)
                new_coord[i] += m_coordinates[i];

            return Step(new_coord);
        }

        Step operator-(const Step &other) const
        {
            if(m_d != other.d())
                throw std::invalid_argument("dimensions do not agree");

            std::vector<int> new_coord = other.coordinates();
            for(int i=0; i<m_d; ++i)
                new_coord[i] -= m_coordinates[i];

            return Step(new_coord);
        }

        Step operator/(const double d) const
        {
            std::vector<int> new_coord = coordinates();
            for(int i=0; i<m_d; ++i)
                new_coord[i] /= d;

            return Step(std::move(new_coord));
        }

        Step& operator+=(const Step &other)
        {
            if(m_d != other.d())
                throw std::invalid_argument("dimensions do not agree");

            std::vector<int> other_coord = other.coordinates();
            for(int i=0; i<m_d; ++i)
                m_coordinates[i] += other_coord[i];

            return *this;
        }

        Step& operator-=(const Step &other)
        {
            if(m_d != other.d())
                throw std::invalid_argument("dimensions do not agree");

            std::vector<int> other_coord = other.coordinates();
            for(int i=0; i<m_d; ++i)
                m_coordinates[i] -= other_coord[i];

            return *this;
        }

        Step& operator/=(const double d)
        {
            // uncool, since my coordinates are integer
            for(int i=0; i<m_d; ++i)
                m_coordinates[i] = std::round(m_coordinates[i]/d);

            return *this;
        }

        // a point is smaller, if the x value is smaller
        // in case of a tie the smaller y value decides
        // only for 2d, since it is only needed for the 2d andrew algorithm
        bool operator <(const Step &other) const
        {
            return x() < other.x() || (x() == other.x() && y() < other.y());
        }

        friend std::ostream& operator<<(std::ostream& os, const Step &obj)
        {
            os << "(";
            for(auto i : obj.m_coordinates)
                os << i << ", ";
            os << ") ";
            return os;
        }

        friend std::ostream& operator<<(std::ostream& os, const std::vector<Step> &obj)
        {
            os << "[";
            for(auto i : obj)
                os << i << " ";
            os << "]";
            return os;
        }

        friend Step cross(const Step &a, const Step &b);
        friend double dot(const Step &a, const Step &b);

        const int& operator[](std::size_t idx) const { return m_coordinates[idx]; };
        int& operator[](std::size_t idx) { return m_coordinates[idx]; };

        // getter
        const int& d() const { return m_d; };
        const int& x(const int n=0) const { return m_coordinates[n]; };
        const int& y() const { return m_coordinates[1]; }; // specialisation for 2d
        const int& z() const { return m_coordinates[2]; }; // specialisation for 3d
        const std::vector<int>& coordinates() const { return m_coordinates; };

    protected:
        int m_d;
        std::vector<int> m_coordinates;

    private:

};

inline bool operator==(const Step &lhs, const Step &rhs)
{
    if(lhs.m_d != rhs.m_d)
        return false;

    for(int i=0; i<lhs.m_d; ++i)
        if(lhs.m_coordinates[i] != rhs.m_coordinates[i])
            return false;

    return true;
}

Step cross(const Step &a, const Step &b);
double dot(const Step &a, const Step &b);

namespace std {
    template<>
    struct hash<Step>
    {
        public:
            // very simple lcg for hashing int vectors
            // http://stackoverflow.com/a/27216842/1698412
            size_t operator()(const Step &s) const
            {
                std::size_t seed = 0;
                for(auto &i : s.m_coordinates)
                    seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                return seed;
            }
    };
}
