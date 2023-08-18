//=======================================================================
/** @file SPIHT_Enc.hpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * The SPIHT encoder. Unlike the decoder, arithmetic encoding is not performed, but has to be done separately.
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#ifndef SPIHT_Enc_hpp
#define SPIHT_Enc_hpp

#include <iostream>
#include <list>
#include <vector>

#include "../../constants/constants.hpp"
#include "../../utilities/include/Utilities.hpp"
#include "../../utilities/include/types.hpp"

namespace VC_PWQ {

class SPIHT_Enc {
  public:
    SPIHT_Enc();

    void encode(std::vector<int>& data,
                int level,
                std::vector<char>* bitwavmax,
                int maxallocbits,
                std::vector<char>& outstream,
                std::vector<int>& context);

  private:
    void sortingPass(std::list<int>& LIP,
                     std::list<int>& LSP,
                     std::list<pixel>& LIS,
                     int compare,
                     std::vector<int>& data,
                     std::vector<char>& outstream,
                     std::vector<int>& context);
    void static refinementPass(std::list<int>& LSP,
                               int LSP_idx,
                               std::vector<int>& data,
                               std::vector<char>& outstream,
                               std::vector<int>& context,
                               int n);

    auto maxDescendant(pixel p) -> int;
    void initMaxDescendant(std::vector<int>& signal);

    std::vector<int> maxDescendants;
    std::vector<int> maxDescendants1;
};

}  // namespace VC_PWQ

#endif /* SPIHT_Enc_hpp */
