//=======================================================================
/** @file ArithEnc.hpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * The Arithmetic Encoder. It is designed to work in conjuction with the SPIHT-coder.
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#ifndef ArithEnc_hpp
#define ArithEnc_hpp

#include <array>
#include <cmath>
#include <iostream>
#include <vector>

#include "../../constants/constants.hpp"

namespace VC_PWQ {

class ArithEnc {
  public:
    ArithEnc();

    void encode(std::vector<char>* instream, std::vector<int>* context, std::vector<char>* outstream);
    void resetCounter();
    void rescaleCounter();

  private:
    std::array<int, CONTEXTS> counter;
    std::array<int, CONTEXTS> counter_total;
};

}  // namespace VC_PWQ

#endif /* ArithEnc_hpp */
