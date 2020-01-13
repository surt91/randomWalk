#ifndef FASTWLENTROPIC_H
#define FASTWLENTROPIC_H

#include "WangLandau.hpp"

/** Variant of Wang Landau sampling.
 *
 * This variant does not need a flatness criterion for the histogram
 * and will terminate after number of steps, determined by the $f$
 * parameter.
 *
 * See http://arxiv.org/pdf/cond-mat/0701672.pdf
 */
class FastWLEntropic : public WangLandau
{
    public:
        FastWLEntropic(const Cmd &o);
        virtual void run();

    protected:
        std::ofstream oss2;
};

#endif
