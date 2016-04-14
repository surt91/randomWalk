#pragma once

#include "WangLandau.hpp"

class FastWangLandau : public WangLandau
{
    public:
        FastWangLandau(const Cmd &o);
        virtual void run();

    protected:
};
