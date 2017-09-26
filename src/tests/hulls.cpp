#include <catch.hpp>

#include "Cmd.hpp"
#include "RNG.hpp"
#include "stat/stat.hpp"
#include "Logging.hpp"
#include "walker/Walker.hpp"
#include "walker/LatticeWalker.hpp"
#include "simulation/Simulation.hpp"

TEST_CASE( "hull types", "[hull]" ) {
    Cmd o;
    o.benchmark = true;
    o.seedRealization = 13;
    o.seedMC = 42;
    o.wantedObservable = WO_VOLUME;
    o.numWalker = 1;

    o.data_path = "bench.tmp";

    o.steps = 1000000;
    o.type = WT_RANDOM_WALK;

    o.chAlg = CH_NOP;

    double A, L;

    std::unique_ptr<Walker> w;

    SECTION( "2D" ) {
        o.d = 2;
        o.steps = 1000000;

        A = 984338.0;
        L = 3747.1842096012;

        Simulation::prepare(w, o);

        SECTION("Andrews") {
            w->setHullAlgo(CH_ANDREWS);
        }
        SECTION("Andrews + Akl") {
            w->setHullAlgo(CH_ANDREWS_AKL);
        }
        SECTION("Jarvis") {
            w->setHullAlgo(CH_JARVIS);
        }
        SECTION("Jarvis + Akl") {
            w->setHullAlgo(CH_JARVIS_AKL);
        }
        SECTION("qhull") {
            w->setHullAlgo(CH_QHULL);
        }

    }
    SECTION( "3D" ) {
        o.d = 3;
        o.steps = 100000;

        A = 6729163.333333333;
        L = 209560.8119192145;

        Simulation::prepare(w, o);

        SECTION("qhull") {
            w->setHullAlgo(CH_QHULL);
        }
    }

    REQUIRE(w->A() == Approx(A));
    REQUIRE(w->L() == Approx(L));
}
