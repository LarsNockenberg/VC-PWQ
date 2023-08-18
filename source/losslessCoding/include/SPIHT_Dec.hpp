//=======================================================================
/** @file SPIHT_Dec.hpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * The SPIHT decoder. Also performs arithmetic decoding.
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#ifndef SPIHT_Dec_hpp
#define SPIHT_Dec_hpp

#include <iostream>
#include <list>
#include <vector>

#include "../../constants/constants.hpp"
#include "../../utilities/include/Utilities.hpp"
#include "../../utilities/include/types.hpp"
#include "ArithDec.hpp"

namespace VC_PWQ {

class SPIHT_Dec {
  public:
    SPIHT_Dec();

    void decode(std::vector<char>& bitstream,
                size_t pos,
                size_t streamlength,
                std::vector<int>& out,
                int origlength,
                int level,
                double* wavmax,
                int* n_real);

    void resetCounter();

  private:
    void sortingPass(std::list<int>& LIP,
                     std::vector<int>& out,
                     std::list<int>& LSP,
                     std::list<pixel>& LIS,
                     int compare);
    void refinementPass(std::list<int>& LSP, int LSP_idx, int compare, std::vector<int>& out);

    auto getBit(int context) -> int;
    void getBits(std::vector<int>& out, int context);

    ArithDec* arithDec;
    int instream_index = 0;
};

}  // namespace VC_PWQ

#endif /* SPIHT_Dec_hpp */
