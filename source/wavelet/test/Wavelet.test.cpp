//=======================================================================
/** @file Wavelet.test.cpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#include "../include/Wavelet.hpp"

#include <iostream>
#include <vector>

#include <catch2/catch_all.hpp>

TEST_CASE("Wavelet transformation") {

    using VC_PWQ::DWT;
    using VC_PWQ::inv_DWT;

    SECTION("Input/Output test") {
        CHECK(true);
    }
}
