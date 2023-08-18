//=======================================================================
/** @file PsychohapticModel.test.hpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#include "../include/PsychohapticModel.hpp"

#include <iostream>
#include <vector>

#include <catch2/catch_all.hpp>

TEST_CASE("DCT") {

    SECTION("test-input") {
        std::vector<double> in(6, 0);
        in[0] = 1;
        in[2] = 1;
        in[4] = 1;
        std::vector<double> out = VC_PWQ::PsychohapticModel::DCT(in);
        CHECK(out.size() == in.size());
        /*std::cout << "output DCT: " << std::endl;
        for(const auto& o: out) {
            std::cout << o << ", ";
        }
        std::cout << std::endl;*/
    }
}
