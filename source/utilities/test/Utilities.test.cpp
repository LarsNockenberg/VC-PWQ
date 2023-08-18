//=======================================================================
/** @file Utilities.test.hpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#include "../include/Utilities.hpp"

#include <cmath>
#include <iostream>
#include <vector>

#include <catch2/catch_all.hpp>

using VC_PWQ::maxQuant;
using VC_PWQ::uniformQuant;

TEST_CASE("MaxQuant") {

    SECTION("numbers below 1") {
        for (double i = 0; i < 100; i++) {
            double in = i / 100;
            double quant = maxQuant(in, 1, 7);
            CHECK(std::abs(in - quant) < 0.01);
        }
    }

    SECTION("numbers above 1") {
        for (double i = 100; i < 400; i++) {
            double in = i / 100;
            double quant = maxQuant(in, 3, 4);
            CHECK(std::abs(in - quant) < 0.1);
        }
    }
}

TEST_CASE("UniformQuant") {

    SECTION("single value") {
        for (double i = 0; i < 100; i++) {

            double in = i / 100;
            double quant = uniformQuant(in, 1, 7);
            CHECK(std::abs(in - quant) < 0.01);
        }
    }

    SECTION("vector") {
        for (double i = 0; i < 100; i++) {

            std::vector<double> in = {i / 100};
            std::vector<double> out = {0};
            uniformQuant(in, out, 0, 1, 1, 7);
            CHECK(std::abs(in[0] - out[0]) < 0.01);
        }
    }
}
