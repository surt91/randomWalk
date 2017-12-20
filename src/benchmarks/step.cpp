#include <benchmark/benchmark.h>

#include "../Step.hpp"

static void BM_neighbors2(benchmark::State& state) {
    Step<int> a {13, 42};
    while (state.KeepRunning())
        a.neighbors();
}
BENCHMARK(BM_neighbors2);

static void BM_neighbors_diag2(benchmark::State& state) {
    Step<int> a {13, 42};
    while (state.KeepRunning())
        a.neighbors(true);
}
BENCHMARK(BM_neighbors_diag2);

static void BM_neighbors3(benchmark::State& state) {
    Step<int> a {13, 42, 23};
    while (state.KeepRunning())
        a.neighbors();
}
BENCHMARK(BM_neighbors3);

static void BM_neighbors_diag3(benchmark::State& state) {
    Step<int> a {13, 42, 23};
    while (state.KeepRunning())
        a.neighbors(true);
}
BENCHMARK(BM_neighbors_diag3);
