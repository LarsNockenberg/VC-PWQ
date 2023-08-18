//=======================================================================
/** @file Decoder.hpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * This class is the centerpiece of the VC-PWQ decoder.
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#ifndef Decoder_hpp
#define Decoder_hpp

#include <iostream>
#include <vector>

#include "../../constants/constants.hpp"
#include "../../losslessCoding/include/SPIHT_Dec.hpp"
#include "../../utilities/include/Utilities.hpp"
#include "../../wavelet/include/Wavelet.hpp"

namespace VC_PWQ {

static constexpr size_t RESERVE_BLOCKS = 10;
static constexpr size_t MIN_SIZE = 8;

class Decoder {
  public:
    Decoder(int maxChannels = MAXCHANNELS_DEFAULT);

    auto decodeMD(std::vector<char>& bitstream) -> std::vector<std::vector<double>>;
    auto decode1D(std::vector<char>& bitstream) -> std::vector<double>;
    void decodeBlock(std::vector<char>& bitstream, std::vector<double>& sig_dwt);

    [[nodiscard]] auto getFS() const -> int;

  protected:
    auto losslessDecoding(std::vector<char>& bitstream, std::vector<int>& sig_intquant, double& multiplicator) -> int;

    static auto fsDecode(std::vector<char>& bitstream) -> int;
    auto decodeChannels(std::vector<char>& bitstream) const -> int;
    void headerDecoding(std::vector<char>& bitstream);
    auto lengthDecoding(std::vector<char>& bitstream) const -> int;

    SPIHT_Dec spiht;

    int bl = 0;
    int dwtlevel = 0;

  private:
    int channelbits = 0;
    int lengthbits = 0;
    int fs = 0;
};

}  // namespace VC_PWQ

#endif /* Decoder_hpp */
