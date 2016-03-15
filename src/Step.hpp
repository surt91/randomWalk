#pragma once

#include "defines.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <stdexcept>

class Step;
namespace std {
    template<>
    class hash<Step>;
}

class Step
{
    friend class std::hash<Step>;

    public:
        Step()
        {};

        Step(int d, double rn)
              : m_d(d)
        {
            // special for 2D lattice, needs to be fixed
            if(d != 2)
                throw std::invalid_argument("d != 2 not implemented");

            m_coordinates = std::vector<int>(2, 0);
            if(rn < 0.25)
                m_coordinates[0] = 1;
            else if(rn < 0.5)
                m_coordinates[0] = -1;
            else if(rn < 0.75)
                m_coordinates[1] = 1;
            else
                m_coordinates[1] = -1;
        };

        Step(const std::vector<int> &coord)
              : m_d(coord.size()),
                m_coordinates(coord)
              {};

        double angle() const //only 2D projection
        {
            return atan2(m_coordinates[1], m_coordinates[0]);
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

        friend std::ostream& operator<<(std::ostream& os, const Step &obj)
        {
            os << "(";
            for(auto i : obj.m_coordinates)
                os << i << ", ";
            os << ") ";
            return os;
        }

        const int& operator[](std::size_t idx) const { return m_coordinates[idx]; };
        int& operator[](std::size_t idx) { return m_coordinates[idx]; };

        // getter
        const int& d() const { return m_d; };
        const int& x(const int n=0) const { return m_coordinates[n]; };
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

namespace std {
    template<>
    class hash<Step>
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
