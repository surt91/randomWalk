#include <catch.hpp>

#include "../Cmd.hpp"
#include "../RNG.hpp"
#include "../walker/Walker.hpp"
#include "../walker/LatticeWalker.hpp"
#include "../walker/GaussWalker.hpp"
#include "../walker/MultipleWalker.hpp"
#include "../simulation/Simulation.hpp"
#include "../simulation/SimpleSampling.hpp"
#include "../simulation/Metropolis.hpp"


TEST_CASE( "observables", "[walk]" ) {
    Cmd o;
    o.seedRealization = 13;
    UniformRNG rngReal(o.seedRealization);

    o.d = 2;
    o.chAlg = CH_ANDREWS_AKL;
    o.numWalker = 1;

    o.steps = 30;
    o.sweep = o.steps;


    SECTION( "2D" ) {
        o.d = 2;
        LatticeWalker w(o.d, o.steps, rngReal, o.chAlg);

        REQUIRE(w.A() == Approx(21.5));
        REQUIRE(w.L() == Approx(18.6445568889));
        REQUIRE(w.L() > 2*w.maxDiameter());
        REQUIRE(w.maxDiameter() == Approx(7.8102496759));
        REQUIRE(w.r() == Approx(4.472135955));
        REQUIRE(w.r()*w.r() == Approx(w.r2()));
        REQUIRE(w.r2() == Approx(20));
        REQUIRE(w.r() >= std::abs(w.rx()));
        REQUIRE(w.r() >= std::abs(w.ry()));
        REQUIRE(w.rx() == Approx(4));
        REQUIRE(w.ry() == Approx(-2));
        REQUIRE(w.num_on_hull() == 7);
        REQUIRE(w.oblateness() == Approx(2.2608617483));
        REQUIRE(w.length() == o.steps);
        REQUIRE(w.visitedSites() == 19);
        REQUIRE(w.enclosedSites() == 20);
    }
    SECTION( "3D" ) {
        o.d = 3;
        o.chAlg = CH_QHULL;
        LatticeWalker w(o.d, o.steps, rngReal, o.chAlg);

        REQUIRE(w.A() == Approx(10.5));
        REQUIRE(w.L() == Approx(29.7580305373));
        REQUIRE(w.L() > 2*w.maxDiameter());
        REQUIRE(w.maxDiameter() == Approx(4.582575695));
        REQUIRE(w.r() == Approx(3.4641016151));
        REQUIRE(w.r()*w.r() == Approx(w.r2()));
        REQUIRE(w.r2() == Approx(12));
        REQUIRE(w.r() >= std::abs(w.rx()));
        REQUIRE(w.r() >= std::abs(w.ry()));
        REQUIRE(w.rx() == Approx(2));
        REQUIRE(w.ry() == Approx(2));
        REQUIRE(w.num_on_hull() == 11);
        REQUIRE(w.oblateness() == Approx(2.2912878475));
        REQUIRE(w.length() == Approx(o.steps));
        REQUIRE(w.visitedSites() == 22);
        REQUIRE(w.enclosedSites() == 22);
    }
    SECTION( "Multi" ) {
        o.d = 2;
        o.numWalker = 3;
        MultipleWalker<LatticeWalker> w(o.d, o.steps, o.numWalker, rngReal, o.chAlg);

        REQUIRE(w.A() == Approx(52.5));
        REQUIRE(w.L() == Approx(29.3203279176));
        REQUIRE(w.maxDiameter() == 0); // not implemented
        REQUIRE(w.r() == 0); // not implemented
        REQUIRE(w.r2() == 0); // not implemented
        REQUIRE(w.rx() == 0); // not implemented
        REQUIRE(w.ry() == 0); // not implemented
        REQUIRE(w.num_on_hull() == 0); // not implemented
        REQUIRE(w.oblateness() == 0); // not implemented
        REQUIRE(w.length() == 0); // not implemented
        REQUIRE(w.visitedSites() == 0); // not implemented
        REQUIRE(w.enclosedSites() == 0); // not implemented
    }
    SECTION( "off-lattice" ) {
        o.d = 2;
        GaussWalker w(o.d, o.steps, rngReal, o.chAlg);

        REQUIRE(w.A() == Approx(21.8773286989));
        REQUIRE(w.L() == Approx(18.426591087));
        REQUIRE(w.L() > 2*w.maxDiameter());
        REQUIRE(w.maxDiameter() == Approx(7.0414677753));
        REQUIRE(w.r() == Approx(4.0898932942));
        REQUIRE(w.r()*w.r() == Approx(w.r2()));
        REQUIRE(w.r2() == Approx(16.7272271581));
        REQUIRE(w.r() >= std::abs(w.rx()));
        REQUIRE(w.r() >= std::abs(w.ry()));
        REQUIRE(w.rx() == Approx(-2.2268209858));
        REQUIRE(w.ry() == Approx(3.4305240788));
        REQUIRE(w.num_on_hull() == 7);
        REQUIRE(w.oblateness() == Approx(1.4028496121));
        REQUIRE(w.length() == Approx(40.3715617656));
        REQUIRE(w.visitedSites() == -1); // not implemented
        REQUIRE(w.enclosedSites() == -1); // not implemented
    }
}
