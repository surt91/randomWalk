#include <catch.hpp>
#include "../Cmd.hpp"

TEST_CASE( "cmd", "[tools]" ) {
    SECTION( "cmd" ) {
        int argc = 6;
        char const *argv[] = {"./randomwalk", "-t", "1", "-N", "200", "-q"};

        Cmd o(argc, argv);

        REQUIRE(o.steps == 200);
        REQUIRE(o.type == WT_RANDOM_WALK);
    }
}
