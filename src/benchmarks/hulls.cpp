#include <benchmark/benchmark.h>

#include "../Cmd.hpp"
#include "../walker/Walker.hpp"
#include "../walker/LatticeWalker.hpp"
#include "../simulation/Simulation.hpp"

template <class ...ExtraArgs>
void BM_convex_hull_algorithm(benchmark::State& state, hull_algorithm_t type, int d=2, walk_type_t wtype=WT_RANDOM_WALK) {
    Cmd o;

    o.d = d;
    o.steps = state.range(0);
    o.type = wtype;

    o.chAlg = CH_NOP;

    std::unique_ptr<Walker> w;
    Simulation::prepare(w, o);

    while (state.KeepRunning())
        w->setHullAlgo(type);
}
BENCHMARK_CAPTURE(BM_convex_hull_algorithm, Andrews, CH_ANDREWS)->Arg(512)->Arg(2048);
BENCHMARK_CAPTURE(BM_convex_hull_algorithm, qhull, CH_QHULL)->Arg(512)->Arg(2048);
BENCHMARK_CAPTURE(BM_convex_hull_algorithm, Jarvis, CH_JARVIS)->Arg(512)->Arg(2048);

BENCHMARK_CAPTURE(BM_convex_hull_algorithm, AndrewsAkl, CH_ANDREWS_AKL)->Arg(512)->Arg(2048);
BENCHMARK_CAPTURE(BM_convex_hull_algorithm, qhullAkl, CH_QHULL_AKL)->Arg(512)->Arg(2048);
BENCHMARK_CAPTURE(BM_convex_hull_algorithm, JarvisAkl, CH_JARVIS_AKL)->Arg(512)->Arg(2048);

BENCHMARK_CAPTURE(BM_convex_hull_algorithm, qhull3d, CH_QHULL, 3)->Arg(512)->Arg(2048);
BENCHMARK_CAPTURE(BM_convex_hull_algorithm, qhullAkl3d, CH_QHULL_AKL, 3)->Arg(512)->Arg(2048);

BENCHMARK_CAPTURE(BM_convex_hull_algorithm, Andrews_gauss, CH_ANDREWS, 2, WT_GAUSSIAN_RANDOM_WALK)->Arg(512)->Arg(2048);
BENCHMARK_CAPTURE(BM_convex_hull_algorithm, qhull_gauss, CH_QHULL, 2, WT_GAUSSIAN_RANDOM_WALK)->Arg(512)->Arg(2048);
BENCHMARK_CAPTURE(BM_convex_hull_algorithm, Jarvis_gauss, CH_JARVIS, 2, WT_GAUSSIAN_RANDOM_WALK)->Arg(512)->Arg(2048);

BENCHMARK_CAPTURE(BM_convex_hull_algorithm, AndrewsAkl_gauss, CH_ANDREWS_AKL, 2, WT_GAUSSIAN_RANDOM_WALK)->Arg(512)->Arg(2048);
BENCHMARK_CAPTURE(BM_convex_hull_algorithm, qhullAkl_gauss, CH_QHULL_AKL, 2, WT_GAUSSIAN_RANDOM_WALK)->Arg(512)->Arg(2048);
BENCHMARK_CAPTURE(BM_convex_hull_algorithm, JarvisAkl_gauss, CH_JARVIS_AKL, 2, WT_GAUSSIAN_RANDOM_WALK)->Arg(512)->Arg(2048);

BENCHMARK_CAPTURE(BM_convex_hull_algorithm, qhull3d_gauss, CH_QHULL, 3, WT_GAUSSIAN_RANDOM_WALK)->Arg(512)->Arg(2048);
BENCHMARK_CAPTURE(BM_convex_hull_algorithm, qhullAkl3d_gauss, CH_QHULL_AKL, 3, WT_GAUSSIAN_RANDOM_WALK)->Arg(512)->Arg(2048);
