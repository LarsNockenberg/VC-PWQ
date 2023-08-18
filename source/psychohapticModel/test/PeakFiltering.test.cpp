//=======================================================================
/** @file PeakFiltering.test.cpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#include "../include/PeakFiltering.hpp"

#include <iostream>
#include <vector>

#include <catch2/catch_all.hpp>

TEST_CASE("FindPeaks") {

    using VC_PWQ::PeakFiltering::FindPeaks;
    using VC_PWQ::PeakFiltering::peak;

    static constexpr size_t bl = 512;

    SECTION("general test") {
        std::vector<double> spectrum(bl, 0);
        spectrum[20] = 4;                                  // NOLINT
        spectrum[45] = 13;                                 // NOLINT
        std::vector<peak> p = FindPeaks(spectrum, 12, 5);  // NOLINT
        CHECK(p.size() == 1);
        CHECK(p.at(0).location == 45);
        CHECK(p.at(0).height == 13);
    }
}

// NOLINTNEXTLINE [readability-function-cognitive-complexity]
TEST_CASE("FindAllPeakLocations") {

    using VC_PWQ::PeakFiltering::FindAllPeakLocations;
    using VC_PWQ::PeakFiltering::peak;

    static constexpr size_t bl = 512;

    SECTION("peaks at first and last position are not detected") {
        std::vector<double> spectrum(bl, 0);
        spectrum[45] = 4;       // NOLINT
        spectrum[0] = 14;       // NOLINT
        spectrum[bl - 1] = 14;  // NOLINT

        std::vector<peak> p = FindAllPeakLocations(spectrum);  // NOLINT
        CHECK(p.size() == 1);
        CHECK(p.at(0).location == 45);
        CHECK(p.at(0).height == 4);
    }

    SECTION("plateau with following peak") {
        std::vector<double> spectrum(bl, 0);
        spectrum[45] = 4;    // NOLINT
        spectrum[46] = 4;    // NOLINT
        spectrum[47] = 4;    // NOLINT
        spectrum[49] = 4.5;  // NOLINT

        std::vector<peak> p = FindAllPeakLocations(spectrum);  // NOLINT
        CHECK(p.size() == 2);
        CHECK(p.at(0).location == 45);  // NOLINT
        CHECK(p.at(0).height == 4);     // NOLINT
        CHECK(p.at(1).location == 49);  // NOLINT
        CHECK(p.at(1).height == 4.5);   // NOLINT
    }

    SECTION("plateau with higher value after it") {
        std::vector<double> spectrum(bl, 0);
        spectrum[50] = 5;  // NOLINT
        spectrum[51] = 5;  // NOLINT
        spectrum[52] = 5;  // NOLINT
        spectrum[53] = 6;  // NOLINT

        std::vector<peak> p = FindAllPeakLocations(spectrum);  // NOLINT
        CHECK(p.size() == 1);
        CHECK(p.at(0).location == 53);
        CHECK(p.at(0).height == 6);
    }
}

TEST_CASE("PeakProminence") {

    using VC_PWQ::PeakFiltering::peak;
    using VC_PWQ::PeakFiltering::PeakProminence;

    static constexpr size_t bl = 512;

    SECTION("peak surrounded by peaks") {
        std::vector<double> spectrum(bl, 0);
        spectrum[1] = 8.2;  // NOLINT
        spectrum[2] = 5;    // NOLINT
        spectrum[3] = 7;    // NOLINT
        spectrum[4] = 6;    // NOLINT
        spectrum[5] = 6.2;  // NOLINT
        spectrum[6] = 8;    // NOLINT

        peak p = {1, 8.2};  // NOLINT
        std::vector<peak> peaks;
        peaks.push_back(p);
        p = {3, 7};  // NOLINT
        peaks.push_back(p);
        p = {6, 8};  // NOLINT
        peaks.push_back(p);

        std::vector<peak> prominences = PeakProminence(spectrum, peaks);

        REQUIRE(prominences.size() == peaks.size());
        CHECK(prominences.at(0).location == 1);
        CHECK(prominences.at(0).height == 8.2);  // NOLINT
        CHECK(prominences.at(1).location == 3);
        CHECK(prominences.at(1).height == 1);
        CHECK(prominences.at(2).location == 6);
        CHECK(prominences.at(2).height == 3);
    }
}

TEST_CASE("FilterPeakCriterion") {

    using VC_PWQ::PeakFiltering::FilterPeakCriterion;
    using VC_PWQ::PeakFiltering::peak;

    static constexpr size_t bl = 512;

    SECTION("two peaks higher, one lower") {

        peak p = {1, 8.2};  // NOLINT
        std::vector<peak> peaks;
        peaks.push_back(p);
        p = {3, 7};  // NOLINT
        peaks.push_back(p);
        p = {6, 8};  // NOLINT
        peaks.push_back(p);

        std::vector<peak> out = FilterPeakCriterion(peaks, 8);  // NOLINT

        REQUIRE(out.size() == 2);
        CHECK(out.at(0).location == 1);
        CHECK(out.at(0).height == 8.2);  // NOLINT
        CHECK(out.at(1).location == 6);
        CHECK(out.at(1).height == 8);
    }
}
