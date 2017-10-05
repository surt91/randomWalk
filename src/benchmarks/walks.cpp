#include <benchmark/benchmark.h>

#include "../Cmd.hpp"
#include "../walker/Walker.hpp"
#include "../walker/LatticeWalker.hpp"
#include "../walker/LoopErasedWalker.hpp"
#include "../walker/SelfAvoidingWalker.hpp"
#include "../walker/RealWalker.hpp"
#include "../walker/GaussWalker.hpp"
#include "../walker/LevyWalker.hpp"
#include "../walker/CorrelatedWalker.hpp"
#include "../simulation/Simulation.hpp"
#include "../simulation/SimpleSampling.hpp"
#include "../simulation/Metropolis.hpp"

double simple_sampling(Cmd o)
{
    o.sampling_method = SM_SIMPLESAMPLING;
    SimpleSampling s(o);
    s.mute();
    s.run();
    return s.check();
}

template <class ...ExtraArgs>
void BM_walk_construction(benchmark::State& state, walk_type_t type, int d=2) {
    Cmd o;

    o.d = d;
    o.steps = state.range(0);
    o.type = type;

    o.chAlg = CH_NOP;

    std::unique_ptr<Walker> w;
    Simulation::prepare(w, o);

    while (state.KeepRunning())
        w->generate_independent_sample();
}

BENCHMARK_CAPTURE(BM_walk_construction, LRW, WT_RANDOM_WALK)->Arg(512)->Arg(2048);
// BENCHMARK_CAPTURE(BM_walk_construction, SAW, WT_SELF_AVOIDING_RANDOM_WALK)->Arg(512)->Arg(2048);
// BENCHMARK_CAPTURE(BM_walk_construction, LERW, WT_LOOP_ERASED_RANDOM_WALK)->Arg(512)->Arg(2048);
