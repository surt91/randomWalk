#pragma once

#include "defines.h"

#include <vector>
#include <iostream>

class Step
{
    public:
        Step()
        {};
        
        Step(int d, double rn)
              : m_d(d)
        {
            // special for 2D lattice, needs to be fixed
            if(d != 2)
                std::cout << ERROR << " d != 2 not implemented";
            
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

