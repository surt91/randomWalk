#include <catch.hpp>

#include "../Cmd.hpp"
#include "../walker/Walker.hpp"
#include "../walker/LatticeWalker.hpp"
#include "../simulation/Simulation.hpp"

TEST_CASE( "hull types", "[hull]" ) {
    Cmd o;
    o.seedRealization = 13;
    o.seedMC = 42;
    o.wantedObservable = WO_VOLUME;
    o.numWalker = 1;

    o.data_path = "bench.tmp";

    o.steps = 1000000;
    o.type = WT_RANDOM_WALK;

    o.chAlg = CH_NOP;

    double A, L;
    int count;

    std::unique_ptr<Walker> w;

    SECTION( "2D" ) {
        o.d = 2;
        o.steps = 1000000;

        A = 984338.0;
        L = 3747.1842096012;
        count = 32;

        Simulation::prepare(w, o);

        SECTION("Andrews") {
            w->setHullAlgo(CH_ANDREWS);
            REQUIRE(w->A() == Approx(A)); REQUIRE(w->L() == Approx(L)); REQUIRE(w->num_on_hull() == count);
        }
        SECTION("Andrews + Akl") {
            w->setHullAlgo(CH_ANDREWS_AKL);
            REQUIRE(w->A() == Approx(A)); REQUIRE(w->L() == Approx(L)); REQUIRE(w->num_on_hull() == count);
        }
        SECTION("Jarvis") {
            w->setHullAlgo(CH_JARVIS);
            REQUIRE(w->A() == Approx(A)); REQUIRE(w->L() == Approx(L)); REQUIRE(w->num_on_hull() == count);
        }
        SECTION("Jarvis + Akl") {
            w->setHullAlgo(CH_JARVIS_AKL);
            REQUIRE(w->A() == Approx(A)); REQUIRE(w->L() == Approx(L)); REQUIRE(w->num_on_hull() == count);
        }
        SECTION("qhull") {
            w->setHullAlgo(CH_QHULL);
            REQUIRE(w->A() == Approx(A)); REQUIRE(w->L() == Approx(L)); REQUIRE(w->num_on_hull() == count);
        }
        SECTION("qhull + Akl") {
            w->setHullAlgo(CH_QHULL_AKL);
            REQUIRE(w->A() == Approx(A)); REQUIRE(w->L() == Approx(L)); REQUIRE(w->num_on_hull() == count);
        }
        SECTION("chan") {
            w->setHullAlgo(CH_CHAN);
            REQUIRE(w->A() == Approx(A)); REQUIRE(w->L() == Approx(L)); REQUIRE(w->num_on_hull() == count);
        }
    }
    SECTION( "3D" ) {
        o.d = 3;
        o.steps = 100000;

        A = 6729163.333333333;
        L = 209560.8119192145;
        count = 124;

        Simulation::prepare(w, o);

        SECTION("qhull") {
            w->setHullAlgo(CH_QHULL);
            REQUIRE(w->A() == Approx(A)); REQUIRE(w->L() == Approx(L)); REQUIRE(w->num_on_hull() == count);
        }
        SECTION("qhull + Akl") {
            w->setHullAlgo(CH_QHULL_AKL);
            REQUIRE(w->A() == Approx(A)); REQUIRE(w->L() == Approx(L)); REQUIRE(w->num_on_hull() == count);
        }
    }
}
