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
#include "../walker/RunAndTumbleWalker.hpp"
#include "../walker/RunAndTumbleWalkerT.hpp"
#include "../walker/ReturningLatticeWalker.hpp"
#include "../simulation/Simulation.hpp"
#include "../simulation/SimpleSampling.hpp"
#include "../simulation/Metropolis.hpp"

#define DO(simple, mcmc) REQUIRE( simple_sampling(o) == Approx(simple) ); REQUIRE( MCMC_sampling(o) == Approx(mcmc) );

double simple_sampling(Cmd o)
{
    o.sampling_method = SM_SIMPLESAMPLING;
    SimpleSampling s(o);
    s.mute();
    s.run();
    return s.check();
}
double MCMC_sampling(Cmd o)
{
    o.sampling_method = SM_METROPOLIS;
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
            DO(17.125, 27.365)
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            DO(17.4983333333, 33.9366666667)
        }
        // TODO also test degenerate cases
    }
    SECTION( "SAW" ) {
        o.type = WT_SELF_AVOIDING_RANDOM_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            DO(52.22, 66.035)
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            DO(47.9533333333, 62.3283333333)
        }
        SECTION( "4D" ) {
            o.d = 4;
            o.chAlg = CH_QHULL;
            DO(27.78875, 29.33875)
        }
    }
    SECTION( "LERW" ) {
        o.type = WT_LOOP_ERASED_RANDOM_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            DO(49.785, 65.3)
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            DO(41.9783333333, 64.8066666667)
        }
    }
    SECTION( "SKSAW" ) {
        o.type = WT_ESCAPE_RANDOM_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            DO(39.125, 53.44)
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            DO(32.3766666667, 53.845)
        }
    }
    SECTION( "Gauss" ) {
        o.type = WT_GAUSSIAN_RANDOM_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            DO(31.866471759, 185.5809789266)
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            DO(97.3490964514, 3737.0787245416)
        }
    }
    SECTION( "Real" ) {
        o.type = WT_REAL_RANDOM_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            DO(16.7933274792, 50.9012134021)
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            DO(19.2329672236, 30.3348949734)
        }
    }
    // SECTION( "Levy" ) {
    //     o.type = WT_LEVY_FLIGHT;
    //     SECTION( "2D" ) {
    //         o.d = 2;
            // ? mcmc = 11251.5984407024
            // DO(775.1929289234, 49233.6337909333)
    //     }
    //     SECTION( "3D" ) {
    //         o.d = 3;
    //         o.chAlg = CH_QHULL;
            // ? mcmc = 2695113.9654559563
            // DO(257.8045513253, 2695113.9654559563)
    //     }
    // }
    SECTION( "Correlated" ) {
        o.type = WT_CORRELATED_RANDOM_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            DO(12.6226044285, 17.1042609018)
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            DO(9.8537841832, 13.3748261316)
        }
    }
    SECTION( "Agent" ) {
        o.type = WT_SCENT_RANDOM_WALK;
        o.d = 2;
        o.width = 10;
        o.tas = 100;
        o.steps = 100;
        o.theta = -5;
        o.numWalker = 5;
        o.gp_path = "out";
        o.svg_path = "out.svg";
        SECTION( "2D random" ) {
            o.agent_start = AS_RANDOM;
            DO(16.335, 50.255)
        }
        SECTION( "2D circle" ) {
            o.agent_start = AS_CIRCLE;
            DO(17.755, 27.17)
        }
        SECTION( "2D triangular lattice" ) {
            o.agent_start = AS_TRIANGULAR;
            DO(14.525, 18.56)
        }
        SECTION( "2D relaxed" ) {
            o.agent_start = AS_RELAXED;
            // FIXME: the second value should be larger than the first!
            DO(14.97, 6.095)
        }
    }
    SECTION( "'True' Self-Avoiding" ) {
        o.type = WT_TRUE_SELF_AVOIDING_WALK;
        SECTION( "2D, beta = 1" ) {
            o.d = 2;
            o.beta = 1.0;
            DO(26.325, 40.32)
        }
        SECTION( "2D, beta = 0.1" ) {
            o.d = 2;
            o.beta = 0.1;
            DO(17.905, 29.23)
        }
        SECTION( "2D, beta = 10" ) {
            o.d = 2;
            o.beta = 10.0;
            DO(36.88, 56.42)
        }
        SECTION( "3D, beta = 1.0" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            o.beta = 1.0;
            DO(26.2783333333, 47.575)
        }
        SECTION( "3D, beta = 0.1" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            o.beta = 0.1;
            DO(18.61, 35.0116666667)
        }
        SECTION( "3D, beta = 10" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            o.beta = 10.0;
            DO(32.1266666667, 53.6433333333)
        }
    }
    SECTION( "Resetting" ) {
        o.type = WT_RESET_WALK;
        o.resetrate = 0.2;
        SECTION( "2D" ) {
            o.d = 2;
            DO(11.08, 14.83)
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            DO(7.8416666667, 10.3333333333)
        }
    }
    SECTION( "Branching" ) {
        o.type = WT_BRANCH_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            DO(13.8732059507, 15.2605265458)
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            DO(118.2958629731, 130.1254492704)
        }
    }
    SECTION( "run-and-tumble" ) {
        o.type = WT_RUNANDTUMBLE_WALK;
        SECTION( "2D, gamma = 1" ) {
            o.d = 2;
            o.gamma = 1.0;
            DO(30.2189224613, 1108.8955173997)
        }
        SECTION( "2D, gamma = 0.5" ) {
            o.d = 2;
            o.gamma = 0.5;
            DO(120.8756898452, 8018.9229082423)
        }
        SECTION( "3, gamma = 1" ) {
            o.d = 3;
            o.gamma = 1.0;
            o.chAlg = CH_QHULL;
            DO(35.8831267428, 437.2901145499)
        }
        SECTION( "3, gamma = 0.5" ) {
            o.d = 3;
            o.gamma = 0.5;
            o.chAlg = CH_QHULL;
            DO(287.0650139422, 119367.451631576)
        }
    }
    SECTION( "run-and-tumble, fixed t" ) {
        o.type = WT_RUNANDTUMBLE_T_WALK;
        o.total_length = o.steps;
        SECTION( "2D, gamma = 1" ) {
            o.d = 2;
            o.gamma = 1.0;
            DO(26.7436202353, 55.8622655703)
        }
        SECTION( "2D, gamma = 0.5" ) {
            o.d = 2;
            o.gamma = 0.5;
            DO(44.9899649586, 88.4146502402)
        }
        SECTION( "3D, gamma = 1" ) {
            o.d = 3;
            o.gamma = 1.0;
            o.chAlg = CH_QHULL;
            DO(33.7755411132, 78.0667975388)
        }
        SECTION( "3D, gamma = 0.5" ) {
            o.d = 3;
            o.gamma = 0.5;
            o.chAlg = CH_QHULL;
            DO(54.3861179925, 166.7646193637)
        }
    }
    SECTION( "Returning RW" ) {
        o.type = WT_RETURNING_LATTICE_WALK;
        SECTION( "2D" ) {
            o.d = 2;
            DO(12.135, 16.105)
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            DO(10.73, 13.9866666667)
        }
    }
    SECTION( "Gauss Resetting" ) {
        o.type = WT_GAUSSIAN_RESET_WALK;
        o.resetrate = 0.2;
        SECTION( "2D" ) {
            o.d = 2;
            DO(26.515668687, 236.1484821327)
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            DO(65.8120971397, 2635.2839025648)
        }
    }
    SECTION( "Resetting Brownian motion" ) {
        o.type = WT_BROWNIAN_RESET_WALK;
        o.resetrate = 0.2;
        o.total_length = 30;
        // steps == total time is identical to gaussian resetting
        SECTION( "2D" ) {
            o.d = 2;
            DO(26.515668687, 62.9871258023)
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            DO(65.8120971397, 384.9845397304)
        }
        // a high number of steps will approximate Brownian motion
        o.steps = 300;
        SECTION( "2D" ) {
            o.d = 2;
            DO(33.6665845282, 69.1591873613)
        }
        SECTION( "3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            DO(117.4722305731, 786.9590268349)
        }
    }
    SECTION( "Multi" ) {
        o.type = WT_RANDOM_WALK;
        o.numWalker = 3;
        o.d = 2;
        DO(58.125, 361.53)
    }
}

TEST_CASE( "bounds", "[walk]" ) {
    Cmd o;
    o.seedRealization = 13;
    o.seedMC = 42;
    o.numWalker = 1;

    o.data_path = "bench.tmp";

    o.d = 2;
    o.chAlg = CH_QHULL;
    o.iterations = 100;

    o.steps = 102;

    o.type = WT_RANDOM_WALK;

    SECTION( "RW: A" ) {
        o.wantedObservable = WO_VOLUME;
        REQUIRE( Simulation::getLowerBound(o) == 0 );
        REQUIRE( Simulation::getUpperBound(o) == std::pow(o.steps / 2., 2) / 2. );
    }
    SECTION( "RW: L" ) {
        o.wantedObservable = WO_SURFACE_AREA;
        REQUIRE( Simulation::getLowerBound(o) == 2 );
        REQUIRE( Simulation::getUpperBound(o) == 2*o.steps );
    }
    SECTION( "RW: V" ) {
        o.d = 3;
        o.wantedObservable = WO_VOLUME;
        REQUIRE( Simulation::getLowerBound(o) == 0 );
        REQUIRE( Simulation::getUpperBound(o) == Approx(std::pow(o.steps / 3., 3) / 6.) );
    }
    SECTION( "RW: dV" ) {
        o.d = 3;
        o.wantedObservable = WO_SURFACE_AREA;
        REQUIRE( Simulation::getLowerBound(o) == 0 );
        REQUIRE( Simulation::getUpperBound(o) == Approx(std::pow(o.steps / 2., 2) / 2.) );
    }
}

TEST_CASE( "misc", "[walk]" ) {
    UniformRNG rngReal(42);

    SECTION( "Dimerization" ) {
        SelfAvoidingWalker w(2, 30, rngReal, CH_ANDREWS_AKL, true);
        w.generate_from_dimerization();
        REQUIRE( w.A() == 52.5 );
    }

    SECTION( "Run-and-tumble fixed-t" ) {
        double t = 103.4;
        RunAndTumbleWalkerT w(2, 30, rngReal, CH_ANDREWS_AKL, true);
        w.setP2(t);
        w.reconstruct();
        REQUIRE( w.length() == t );
    }

    double r = 0.1;
    SECTION( "Resetting Walk (Lattice)" ) {
        ResetWalker w1(2, 30, rngReal, CH_ANDREWS_AKL, true);
        w1.setP1(r);
        w1.reconstruct();
        REQUIRE( w1.num_resets() == 3 );
        REQUIRE( w1.maxsteps_partialwalk() == 13 );
    }
    SECTION( "Resetting Walk (Gaussian)" ) {
        GaussResetWalker w2(2, 30, rngReal, CH_ANDREWS_AKL, true);
        w2.setP1(r);
        w2.reconstruct();
        REQUIRE( w2.num_resets() == 4 );
        REQUIRE( w2.maxsteps_partialwalk() == 12 );
    }
    SECTION( "Resetting Walk (Brownian)" ) {
        BrownianResetWalker w3(2, 30, rngReal, CH_ANDREWS_AKL, true);
        w3.setP1(r);
        w3.reconstruct();
        REQUIRE( w3.num_resets() == 4 );
        REQUIRE( w3.maxsteps_partialwalk() == 12 );
    }

    SECTION( "Closed Walk" ) {
        ReturningLatticeWalker w(2, 30, rngReal, CH_ANDREWS_AKL, true);
        w.reconstruct();
        Step<int> s = Step<int>(w.d);
        for(auto i : w.steps())
            s += i;
        REQUIRE( s == Step<int>(w.d) );
    }
}
