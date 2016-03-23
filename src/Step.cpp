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

// measures the parallelness of two lines defined by three points
// 0 means the lines are orthogonal
// 1 means the lines are parallel
// -1 means antiparallel
// works with a dot product of the normalized vectors
double parallelness(const Step &a, const Step &b, const Step &c)
{
    Step l1 = b-a;
    Step l2 = c-b;

    double ll1 = l1.length();
    double ll2 = l2.length();

    double p = 0;
    for(int i=0; i<l1.m_d; ++i)
        p += l1[i]/ll1 * l2[i]/ll2;

    return p;
}
