//=======================================================================
/** @file ArithDec.hpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * The Arithmetic Decoder. It is designed to work in conjuction with the SPIHT-coder.
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#ifndef ARITHDEC_HPP
#define ARITHDEC_HPP

#include <array>
#include <cmath>
#include <iostream>
#include <vector>

#include "../../constants/constants.hpp"

namespace VC_PWQ {

static constexpr int DIGITS_START = 10;
static constexpr int SHIFT = DIGITS_START - 1;

class ArithDec {
  public:
    ArithDec();

    void initDecoding(std::vector<char>* instream, size_t pos, size_t length);
    auto decode(int context) -> int;
    void resetCounter();
    void rescaleCounter();

  private:
    std::vector<char>* instream;
    size_t in_index;
    size_t max_index;

    int range_diff;
    int range_lower;
    int range_upper;

    std::array<int, CONTEXTS> counter;
    std::array<int, CONTEXTS> counter_total;
    int in_leading;
};

}  // namespace VC_PWQ

#endif /* ARITHDEC_HPP */
