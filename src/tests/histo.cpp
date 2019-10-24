#include <catch.hpp>
#include "../simulation/Histogram.hpp"
#include "../stat/HistogramND.hpp"

TEST_CASE( "histograms", "[tools]" ) {
    SECTION( "histogram" ) {
        Histogram h(10, 1, 5);

        h.add(0);
        h.add(2, 3);
        h.add(3.14152);
        REQUIRE(h.sum() == 4);
        REQUIRE(h.count() == 2);
        REQUIRE(h[3.2] == 1);
        REQUIRE(h[2] == 3);
        REQUIRE(h.at(0) == 0);
        REQUIRE(h.get_num_bins() == 10);
        REQUIRE(h.min() == 0);
        REQUIRE(h.mean() == Approx(0.4));

        h.reset();
        REQUIRE(h.sum() == 0);
        REQUIRE(h.count() == 0);

        h.add(0);
        h.add(2, 3);
        h.add(3.14152);
        h.trim();
        REQUIRE(h.get_num_bins() == 4);
        REQUIRE(h.at(0) == 3);
        REQUIRE(h[2] == 3);
        REQUIRE(h.at(3) == 1);
        REQUIRE(h[3.2] == 1);
        REQUIRE(h.min() == 0);

        auto s = h.ascii_table();
        REQUIRE(s.size() > 0);
    }

    SECTION( "histogramND" ) {
        HistogramND h(16, 2, 1, 5);

        std::vector<double> p1({1.5, 1.5});
        std::vector<double> p2({2.5, 1.5});
        std::vector<double> p3({3.5, 1.5});
        h.add(p1);
        h.add(p2);
        h.add(p3);
        h.add(p3);
        REQUIRE(h.sum() == 4);
        REQUIRE(h.max() == 2);

        h.reset();
        REQUIRE(h.sum() == 0);
        REQUIRE(h.max() == 0);


        REQUIRE(h.num_bins() == 16);
    }
}
