#pragma once

#include "WangLandau.hpp"

class FastWangLandau : public WangLandau
{
    public:
        FastWangLandau(const Cmd &o);
        virtual ~FastWangLandau() {};
        virtual void run();

    protected:
};
