#pragma once

#include "WangLandau.hpp"

/** Variant of Wang Landau sampling.
 *
 * This variant does not need a flatness criterion for the histogram
 * and will terminate after number of steps, determined by the $f$
 * parameter.
 *
 * See http://arxiv.org/pdf/cond-mat/0701672.pdf
 */
class FastWangLandau : public WangLandau
{
    public:
        FastWangLandau(const Cmd &o);
        virtual ~FastWangLandau() {};
        virtual void run();

    protected:
};
