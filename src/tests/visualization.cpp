#include <catch.hpp>

#include <stdio.h>

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

    SECTION( "Gauss change Example" ) {
        GaussWalker w(o.d, o.steps, rngReal, o.chAlg);
        filename = "bench.gauss";
        remove(filename.c_str());
        w.svgOfChange(filename, rngReal);
    }

    SECTION( "SAW Pivot Example" ) {
        SelfAvoidingWalker w(o.d, o.steps, rngReal, o.chAlg);
        filename = "bench.saw";
        remove(filename.c_str());
        w.svgOfPivot(filename);
    }

    SECTION( "LERW Example" ) {
        LoopErasedWalker w(o.d, o.steps, rngReal, o.chAlg);
        filename = "bench.lerw";
        remove(filename.c_str());
        w.svgOfErasedLoops(filename);
    }

    SECTION( "2D" ) {
        o.d = 2;
        LatticeWalker w(o.d, o.steps, rngReal, o.chAlg);
        SECTION( "SVG" ) {
            filename = "bench.svg";
            remove(filename.c_str());
            w.svg(filename, true);
        }
        SECTION( "GP" ) {
            filename = "bench2.gp";
            remove(filename.c_str());
            w.gp("bench2", true);
        }
    }

    SECTION( "3D" ) {
        o.d = 3;
        o.chAlg = CH_QHULL;
        LatticeWalker w(o.d, o.steps, rngReal, o.chAlg);
        SECTION( "SVG" ) {
            filename = "bench3.svg";
            remove(filename.c_str());
            w.svg(filename, true);
        }
        SECTION( "POV" ) {
            filename = "bench.pov";
            remove(filename.c_str());
            w.pov(filename, true);
        }
        SECTION( "GP" ) {
            filename = "bench3.gp";
            remove(filename.c_str());
            w.gp("bench3", true);
        }
        SECTION( "three.js" ) {
            filename = "bench.tjs";
            remove(filename.c_str());
            w.threejs(filename, true);
        }
    }

    SECTION( "Multi" ) {
        o.d = 2;
        o.numWalker = 3;
        MultipleWalker<LatticeWalker> w(o.d, o.steps, o.numWalker, rngReal, o.chAlg);
        SECTION( "SVG" ) {
            filename = "benchM.svg";
            remove(filename.c_str());
            w.svg(filename, true);
        }
        SECTION( "GP" ) {
            filename = "benchM.gp";
            remove(filename.c_str());
            w.gp("benchM", true);
        }
        SECTION( "GP 3D" ) {
            o.d = 3;
            o.chAlg = CH_QHULL;
            o.numWalker = 3;
            MultipleWalker<GaussWalker> w(o.d, o.steps, o.numWalker, rngReal, o.chAlg);
            filename = "benchM3.gp";
            remove(filename.c_str());
            w.gp("benchM3", true);
        }
    }

    SECTION( " Scent") {
        o.d = 2;
        o.width = 10;
        o.tas = 100;
        o.steps = 100;
        o.theta = -1;
        o.numWalker = 5;
        ScentWalker w(o.d, o.steps, o.numWalker, o.width, o.tas, rngReal, o.chAlg, true, true);
        SECTION( "SVG" ) {
            filename = "benchHist.svg";
            remove(filename.c_str());
            w.svg(filename);
        }
        SECTION( "gp" ) {
            filename = "benchHist.gp";
            remove(filename.c_str());
            w.gp("benchHist", true);
        }
    }

    REQUIRE( filesize(filename.c_str()) > 200 );
}
