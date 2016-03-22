#include "Step.hpp"

Step cross(const Step &a, const Step &b)
{
    if(a.m_d != b.m_d)
        throw std::invalid_argument("dimensions do not agree");
    if(a.m_d != 3)
        throw std::invalid_argument("cross product only defined for d=3");

    std::vector<int> ret(3);
    ret[0] = a.y()*b.z() - a.z()*b.y();
    ret[1] = a.z()*b.x() - a.x()*b.z();
    ret[2] = a.x()*b.y() - a.y()*b.x();
    return Step(ret);
}

double dot(const Step &a, const Step &b)
{
    if(a.m_d != b.m_d)
        throw std::invalid_argument("dimensions do not agree");

    double p = 0;

    for(int i=0; i<a.m_d; ++i)
        p += a[i]*b[i];

    return p;
}
