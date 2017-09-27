#include <catch.hpp>

#include "../Cmd.hpp"
#include "../walker/LatticeWalker.hpp"
#include "../walker/LoopErasedWalker.hpp"
#include "../walker/SelfAvoidingWalker.hpp"
#include "../simulation/Simulation.hpp"

// these tests will just draw images but not compare them
// it just ensures that the files are not empty: > 200 Bytes
// this will find errors leading to program exit but not functional errors

TEST_CASE( "images", "[vis]" ) {
    Cmd o;
    UniformRNG rngReal(o.seedRealization);
    o.d = 2;
    o.steps = 30;
    o.chAlg = CH_ANDREWS_AKL;
    std::string filename;

    SECTION( "SAW Pivot Example" ) {
        SelfAvoidingWalker w(o.d, o.steps, rngReal, o.chAlg);
        filename = "bench.saw";
        w.svgOfPivot(filename);
    }

    SECTION( "LERW Example" ) {
        LoopErasedWalker w(o.d, o.steps, rngReal, o.chAlg);
        filename = "bench.lerw";
        w.svgOfErasedLoops(filename);
    }

    SECTION( "2D" ) {
        o.d = 2;
        LatticeWalker w(o.d, o.steps, rngReal, o.chAlg);
        SECTION( "SVG" ) {
            filename = "bench.svg";
            w.svg(filename, true);
        }
        SECTION( "GP" ) {
            w.gp("bench2", true);
            filename = "bench2.gp";
        }
    }
    SECTION( "3D" ) {
        o.d = 3;
        o.chAlg = CH_QHULL;
        LatticeWalker w(o.d, o.steps, rngReal, o.chAlg);
        SECTION( "POV" ) {
            filename = "bench.pov";
            w.pov(filename, true);
        }
        SECTION( "GP" ) {
            w.gp("bench3", true);
            filename = "bench3.gp";
        }
        SECTION( "three.js" ) {
            filename = "bench.tjs";
            w.threejs(filename, true);
        }
    }

    REQUIRE( filesize(filename.c_str()) > 200 );
}
