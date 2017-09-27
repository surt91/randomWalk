#include <catch.hpp>

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

double simple_sampling(const Cmd o)
{
    SimpleSampling s(o);
    s.mute();
    s.run();
    return s.check();
}
double MCMC_sampling(const Cmd o)
{
    Metropolis s(o);
    s.mute();
    s.run();
    return s.check();
}

TEST_CASE( "walk types", "[walk]" ) {
    Cmd o;
    o.seedRealization = 13;
    o.seedMC = 42;
    o.wantedObservable = WO_VOLUME;
    o.numWalker = 1;

    o.t_eq = 10;
    o.theta = -10;
    o.simpleSampling = false;
    o.sampling_method = SM_METROPOLIS;

    o.data_path = "bench.tmp";

    o.d = 2;
    o.chAlg = CH_ANDREWS_AKL;
    o.iterations = 100;

    o.steps = 30;
    o.sweep = o.steps;

    double simple, mcmc;

    SECTION( "RW" ) {
        o.type = WT_RANDOM_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            simple = 17.125;
            mcmc = 27.365;
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            simple = 17.4983333333;
            mcmc = 33.9366666667;
        }
    }
    SECTION( "SAW" ) {
        o.type = WT_SELF_AVOIDING_RANDOM_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            simple = 52.22;
            mcmc = 66.035;
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            simple = 47.9533333333;
            mcmc = 62.3283333333;
        }
        SECTION( "4D" ) {
            o.d = 4;
            o.chAlg = CH_QHULL;
            simple = 27.78875;
            mcmc = 29.33875;
        }
    }
    SECTION( "LERW" ) {
        o.type = WT_LOOP_ERASED_RANDOM_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            simple = 47.95;
            mcmc = 65.3;
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            simple = 40.4283333333;
            mcmc = 64.8066666667;
        }
    }
    SECTION( "SKSAW" ) {
        o.type = WT_ESCAPE_RANDOM_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            simple = 39.125;
            mcmc = 53.44;
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            simple = 32.3766666667;
            mcmc = 53.845;
        }
    }
    SECTION( "Gauss" ) {
        o.type = WT_GAUSSIAN_RANDOM_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            simple = 31.866471759;
            mcmc = 185.5809789266;
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            simple = 97.3490964514;
            mcmc = 3737.0787245416;
        }
    }
    SECTION( "Real" ) {
        o.type = WT_REAL_RANDOM_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            simple = 16.3204219374;
            mcmc = 29.1930753626;
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            simple = 16.8672871474;
            mcmc = 31.3519408254;
        }
    }
    SECTION( "Levy" ) {
        o.type = WT_LEVY_FLIGHT;
        SECTION( "2D" ) {
            o.d = 2;
            simple = 775.1929289234;
            mcmc = 49233.6337909333;
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            simple = 257.8045513253;
            mcmc = 419333.7615910646;
        }
    }
    SECTION( "Correlated" ) {
        o.type = WT_CORRELATED_RANDOM_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            simple = 12.6226044285;
            mcmc = 17.1042609018;
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            simple = 9.8537841832;
            mcmc = 13.3748261316;
        }
    }
    SECTION( "Agent" ) {
        o.type = WT_SCENT_RANDOM_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            simple = 17.42;
            mcmc = 20.35;
        }
    }

    REQUIRE( simple_sampling(o) == Approx(simple) );
    REQUIRE( MCMC_sampling(o) == Approx(mcmc) );
}
