#include <catch.hpp>

#include "../Step.hpp"

TEST_CASE("Step is tested", "[step]" ) {
    SECTION( "length" ) {
        Step<int> a({3, 4, 0});

        REQUIRE( a.d() == 3 );
        REQUIRE( a.length() == 5 );
        REQUIRE( a.length2() == 25 );
    }

    SECTION( "turns" ) {
        Step<int> b({1, 0});
        REQUIRE( b.d() == 2 );
        REQUIRE( b.left_turn() == Step<int>({0, 1}) );
        REQUIRE( b.left_turn().left_turn() == Step<int>({-1, 0}) );
        REQUIRE( b.left_turn().left_turn().left_turn() == Step<int>({0, -1}) );
        REQUIRE( b.right_turn() == Step<int>({0, -1}) );
        REQUIRE( b.right_turn().right_turn() == Step<int>({-1, 0}) );
        REQUIRE( b.right_turn().right_turn().right_turn() == Step<int>({0, 1}) );
    }

    SECTION( "neighbors" ) {
        Step<int> c2({0, 0});
        REQUIRE( c2.d() == 2 );
        std::set<Step<int>> direct2 {
            Step<int>({1, 0}), Step<int>({-1, 0}),
            Step<int>({0, 1}), Step<int>({0, -1})
        };
        std::set<Step<int>> diagonal2 {
            Step<int>({1, 1}), Step<int>({-1, 1}),
            Step<int>({1, -1}), Step<int>({-1, -1})
        };
        diagonal2.insert(begin(direct2), end(direct2));
        auto tmp_direct2 = c2.neighbors();
        auto set_direct2 = std::set<Step<int>>(begin(tmp_direct2), end(tmp_direct2));
        REQUIRE( direct2.size() == 4 );
        REQUIRE( set_direct2.size() == 4 );
        REQUIRE( direct2 == set_direct2 );
        auto tmp_diagonal2 = c2.neighbors(true);
        auto set_diagonal2 = std::set<Step<int>>(begin(tmp_diagonal2), end(tmp_diagonal2));
        REQUIRE( diagonal2.size() == 8 );
        REQUIRE( set_diagonal2.size() == 8 );
        REQUIRE( diagonal2.size() == set_diagonal2.size() );
        REQUIRE( diagonal2 == set_diagonal2 );

        Step<int> c3({0, 0, 0});
        REQUIRE( c3.d() == 3 );
        std::set<Step<int>> direct3 {
            Step<int>({1, 0, 0}), Step<int>({-1, 0, 0}),
            Step<int>({0, 1, 0}), Step<int>({0, -1, 0}),
            Step<int>({0, 0, 1}), Step<int>({0, 0, -1})
        };
        std::set<Step<int>> diagonal3 {
            Step<int>({1, 1, 1}),
            Step<int>({-1, 1, 1}),
            Step<int>({1, -1, 1}),
            Step<int>({1, 1, -1}),
            Step<int>({1, -1, -1}),
            Step<int>({-1, 1, -1}),
            Step<int>({-1, -1, 1}),
            Step<int>({-1, -1, -1}),

            Step<int>({1, 1, 0}),
            Step<int>({1, -1, 0}),
            Step<int>({-1, 1, 0}),
            Step<int>({-1, -1, 0}),

            Step<int>({1, 0, 1}),
            Step<int>({1, 0, -1}),
            Step<int>({-1, 0, 1}),
            Step<int>({-1, 0, -1}),

            Step<int>({0, 1, 1}),
            Step<int>({0, 1, -1}),
            Step<int>({0, -1, 1}),
            Step<int>({0, -1, -1})
        };
        diagonal3.insert(begin(direct3), end(direct3));
        auto tmp_direct3 = c3.neighbors();
        auto set_direct3 = std::set<Step<int>>(begin(tmp_direct3), end(tmp_direct3));
        REQUIRE( direct3.size() == 6 );
        REQUIRE( set_direct3.size() == 6 );
        REQUIRE( direct3 == set_direct3 );
        auto tmp_diagonal3 = c3.neighbors(true);
        auto set_diagonal3 = std::set<Step<int>>(begin(tmp_diagonal3), end(tmp_diagonal3));
        REQUIRE( diagonal3.size() == 26 );
        REQUIRE( set_diagonal3.size() == 26 );
        REQUIRE( diagonal3 == set_diagonal3 );
    }
}
