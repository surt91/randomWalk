#include <catch.hpp>

#include "Step.hpp"

TEST_CASE("Step is tested", "[step]" ) {
    Step<int> a({3, 4, 0});

    REQUIRE( a.d() == 3 );
    REQUIRE( a.length() == 5 );
    REQUIRE( a.length2() == 25 );

    Step<int> b({1, 0});
    REQUIRE( b.d() == 2 );
    REQUIRE( b.left_turn() == Step<int>({0, 1}) );
    REQUIRE( b.left_turn().left_turn() == Step<int>({-1, 0}) );
    REQUIRE( b.left_turn().left_turn().left_turn() == Step<int>({0, -1}) );
    REQUIRE( b.right_turn() == Step<int>({0, -1}) );
    REQUIRE( b.right_turn().right_turn() == Step<int>({-1, 0}) );
    REQUIRE( b.right_turn().right_turn().right_turn() == Step<int>({0, 1}) );
}
