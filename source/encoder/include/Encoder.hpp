//=======================================================================
/** @file Encoder.hpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * This class is the centerpiece of the VC-PWQ encoder.
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#ifndef Encoder_hpp
#define Encoder_hpp

#include <cmath>
#include <complex>
#include <iostream>
#include <limits>
#include <vector>

#include "../../constants/constants.hpp"
#include "../../losslessCoding/include/ArithEnc.hpp"
#include "../../losslessCoding/include/SPIHT_Enc.hpp"
#include "../../psychohapticModel/include/PsychohapticModel.hpp"
#include "../../utilities/include/Utilities.hpp"
#include "../../wavelet/include/Wavelet.hpp"

namespace VC_PWQ {

static constexpr size_t MAXSTREAMLENGTH = 2 ^ 14 - 1;
static constexpr size_t BINARY_RESERVE = 20000;

class Encoder {
  public:
    Encoder(int bl_new, int fs_new, int maxChannels = MAXCHANNELS_DEFAULT);

    auto encodeMD(std::vector<std::vector<double>>& sig, int bitbudget) -> std::vector<char>;
    auto encode1D(std::vector<double>& sig, int bitbudget) -> std::vector<char>;

  protected:
    auto encodeBlock(std::vector<double>& block_dwt,
                     std::vector<double> SMR,
                     std::vector<double> bandenergy,
                     std::vector<char>& bitstream,
                     int bitbudget) -> std::vector<double>;

    void losslessEncoding(std::vector<int>& block_intquant,
                          std::vector<char>& bitwavmax,
                          int bitmax,
                          std::vector<char>& bitstream);

    void fsEncode(std::vector<char>* bitstream) const;
    auto encodeChannels(int channels, std::vector<char>* bitstream) const -> int;
    void headerEncoding(std::vector<char>* bitstream) const;
    void lengthEncoding(std::vector<char>& outstream, std::vector<char>& blockstream) const;
    void static maximumWaveletCoefficient(std::vector<double>& sig, double* qwavmax, std::vector<char>* bitwavmax);
    void updateNoise(std::vector<double>& bandenergy,
                     std::vector<double>& noiseenergy,
                     std::vector<double>& SNR,
                     std::vector<double>& MNR,
                     std::vector<double>& SMR) const;

    SPIHT_Enc spiht;
    ArithEnc arithmetic;
    PsychohapticModel pm;

    std::vector<int> book;
    std::vector<int> book_cumulative;
    int l_book;
    int bl;
    int dwtlevel;

  private:
    int channelbits;
    int fs;
    int lengthbits;
};

}  // namespace VC_PWQ

#endif /* Encoder_hpp */
