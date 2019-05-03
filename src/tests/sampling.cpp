#include <catch.hpp>

#include "../Cmd.hpp"
#include "../walker/Walker.hpp"
#include "../walker/LatticeWalker.hpp"
#include "../simulation/Simulation.hpp"
#include "../simulation/SimpleSampling.hpp"
#include "../simulation/Metropolis.hpp"
#include "../simulation/WangLandau.hpp"
#include "../simulation/FastWLEntropic.hpp"
#include "../simulation/MetropolisParallelTempering.hpp"

TEST_CASE( "sampling types", "[sampling]" ) {
    Cmd o;

    o.d = 2;
    o.wantedObservable = WO_VOLUME;
    o.chAlg = CH_ANDREWS_AKL;
    o.iterations = 100;
    o.numWalker = 1;
    o.seedRealization = 13;
    o.seedMC = 42;

    o.t_eq = 0;
    o.t_eqMax = 1000;
    o.simpleSampling = false;
    o.theta = -20;

    o.wangLandauBorders = std::vector<double>({65, 67.5, 70});
    o.wangLandauOverlap = 2;
    o.wangLandauBins = 4;
    o.lnf_min = 10e-3;
    o.flatness_criterion = 0.6;

    o.parallelTemperatures = std::vector<double>({40, 50, 60});

    o.data_path_vector = std::vector<std::string>({"bench1.tmp", "bench2.tmp", "bench3.tmp"});
    o.type = WT_RANDOM_WALK;

    double checksum;

    std::unique_ptr<Simulation> s;

    SECTION( "Simple Sampling" ) {
        o.sampling_method = SM_SIMPLESAMPLING;
        o.steps = 10000;
        o.sweep = o.steps;
        checksum = 7471.585;
        s = std::unique_ptr<SimpleSampling>(new SimpleSampling(o));
    }
    SECTION( "Metropolis" ) {
        o.sampling_method = SM_METROPOLIS;
        o.steps = 100;
        o.sweep = o.steps;
        checksum = 136.155;
        s = std::unique_ptr<Metropolis>(new Metropolis(o));
    }
    SECTION( "Wang landau" ) {
        o.sampling_method = SM_WANG_LANDAU;
        o.iterations = 1;
        o.steps = 100;
        o.sweep = o.steps;
        checksum = 131.0506230333;
        s = std::unique_ptr<WangLandau>(new WangLandau(o));
    }
    SECTION( "Fast Wang landau + entropic" ) {
        o.sampling_method = SM_FAST_WANG_LANDAU;
        o.iterations = 1;
        o.steps = 100;
        o.sweep = o.steps;
        checksum = 137.9641392872;
        s = std::unique_ptr<FastWLEntropic>(new FastWLEntropic(o));
    }
    SECTION( "Parallel Tempering" ) {
        o.sampling_method = SM_METROPOLIS_PARALLEL_TEMPERING;
        o.steps = 100;
        o.sweep = o.steps;
        checksum = 11021.0;
        s = std::unique_ptr<MetropolisParallelTempering>(new MetropolisParallelTempering(o));
    }
    SECTION( "Metropolis Auto-Equilibration" ) {
        o.sampling_method = SM_METROPOLIS;
        o.t_eq = -1;
        o.steps = 100;
        o.sweep = o.steps;
        checksum = 392.17;
        s = std::unique_ptr<Metropolis>(new Metropolis(o));
    }
    s->mute();
    s->run();
    REQUIRE( s->check() == Approx(checksum) );

}
