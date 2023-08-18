//=======================================================================
/** @file Decoder.cpp
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

#include "../include/Decoder.hpp"

namespace VC_PWQ {

/**
 * @brief constructor of the decoder
 * @param maxChannels specify maximum number of channels supported; default on 8
 */
Decoder::Decoder(int maxChannels) : channelbits(ceil(log2(maxChannels + 1))) {}

/**
 * @brief decode multichannel signal
 * @param bitstream bitstream of encoded signal
 * @return decoded multichannel signal
 */
auto Decoder::decodeMD(std::vector<char>& bitstream) -> std::vector<std::vector<double>> {

    std::vector<std::vector<double>> sig_rec;

    int channels = decodeChannels(bitstream);

    spiht.resetCounter();

    fs = fsDecode(bitstream);

    for (int c = 0; c < channels; c++) {
        std::vector<double> sig;
        sig.reserve(MAX_BL * RESERVE_BLOCKS);
        sig_rec.push_back(sig);
    }

    int start = 0;
    while (bitstream.size() > MIN_SIZE) {
        for (int c = 0; c < channels; c++) {
            headerDecoding(bitstream);
            sig_rec.at(c).resize(start + bl);

            std::vector<double> buffer(bl, 0);
            decodeBlock(bitstream, buffer);
            std::vector<double> buffer_out = inv_DWT(buffer, dwtlevel);
            std::copy(buffer_out.begin(), buffer_out.end(), sig_rec.at(c).begin() + start);
        }
        start += bl;
    }

    return sig_rec;
}

/**
 * @brief decode single channel signal
 * @param bitstream bitstream of encoded signal
 * @return decoded signal
 */
auto Decoder::decode1D(std::vector<char>& bitstream) -> std::vector<double> {

    std::vector<double> sig_rec;
    spiht.resetCounter();

    fs = fsDecode(bitstream);
    sig_rec.reserve(MAX_BL * RESERVE_BLOCKS);
    long start = 0;

    while (bitstream.size() > MIN_SIZE) {
        headerDecoding(bitstream);
        sig_rec.resize(start + bl);
        std::vector<double> buffer(bl, 0);
        decodeBlock(bitstream, buffer);
        std::vector<double> buffer_out = inv_DWT(buffer, dwtlevel);
        std::copy(buffer_out.begin(), buffer_out.end(), sig_rec.begin() + start);
        start += bl;
    }
    return sig_rec;
}

/**
 * @brief decode a block, single channel signal
 * @param bitstream bitstream of encoded signal
 * @param sig_dwt decoded block in wavelet domain
 */
void Decoder::decodeBlock(std::vector<char>& bitstream, std::vector<double>& sig_dwt) {
    double multiplicator = 0;

    std::vector<int> sig_intquant(bl, 0);
    int content = losslessDecoding(bitstream, sig_intquant, multiplicator);

    if (content == 1) {

        for (int i = 0; i < bl; i++) {
            sig_dwt[i] = (double)sig_intquant[i] * multiplicator;
        }
    } else {
        for (int i = 0; i < bl; i++) {
            sig_dwt[i] = 0;
        }
    }
}

/**
 * @brief lossless decoding of a block, single channel
 * @param bitstream bitstream of encoded signal
 * @param sig_intquant quantized block, output variable
 * @param multiplicator rescaling value, output variable
 * @return flag indicating if block contains data
 */
auto Decoder::losslessDecoding(std::vector<char>& bitstream, std::vector<int>& sig_intquant, double& multiplicator)
    -> int {

    int start = 0;
    int segmentlength = lengthDecoding(bitstream);

    if (segmentlength > 0) {

        double recwavmax = 0;
        int recbitmax = 0;
        spiht.decode(bitstream, start, segmentlength, sig_intquant, bl, dwtlevel, &recwavmax, &recbitmax);
        multiplicator = recwavmax / (double)(1 << recbitmax);
        bitstream.erase(bitstream.begin(), bitstream.begin() + start + segmentlength);
        return 1;
    }
    return 0;
}

/**
 * @brief decode and return the sampling frequency
 * @param bitstream bitstream of encoded signal
 * @return sampling frequency
 */
auto Decoder::fsDecode(std::vector<char>& bitstream) -> int {

    int fs = 0;
    if (bitstream.at(0) == 0) {
        if (bitstream.at(1) == 0) {
            fs = FS_0;
        } else {
            fs = FS_1;
        }
    } else {
        if (bitstream.at(1) == 0) {
            fs = FS_2;
        } else {
            fs = 0;
        }
    }
    bitstream.erase(bitstream.begin(), bitstream.begin() + 2);
    return fs;
}

/**
 * @brief decode and return the channel count
 * @param bitstream bitstream of encoded signal
 * @return channel count
 */
auto Decoder::decodeChannels(std::vector<char>& bitstream) const -> int {
    int channels = bi2de(&bitstream, channelbits, 0);
    bitstream.erase(bitstream.begin(), bitstream.begin() + channelbits);
    return channels;
}

/**
 * @brief decode the header and set variables in decoder object
 * @param bitstream bitstream of encoded signal
 */
void Decoder::headerDecoding(std::vector<char>& bitstream) {

    lengthbits = LENGTHBITS_4;
    int start = 0;
    if (bitstream.at(start) == 1) {
        bl = BL_0;
        start += 1;
        lengthbits = LENGTHBITS_0;
    } else {
        if (bitstream.at(start + 1) == 1) {
            bl = BL_1;
            start += 2;
            lengthbits = LENGTHBITS_1;
        } else {
            if (bitstream.at(start + 2) == 1) {
                bl = BL_2;
                start += 3;
                lengthbits = LENGTHBITS_2;
            } else {
                if (bitstream.at(start + 3) == 0) {
                    bl = BL_3;
                    start += 4;
                    lengthbits = LENGTHBITS_3;
                } else {
                    bl = BL_4;
                    start += 4;
                }
            }
        }
    }

    dwtlevel = (int)log2(bl) - 2;

    bitstream.erase(bitstream.begin(), bitstream.begin() + start);
}

/**
 * @brief decode and return the length of a binary encoded signal block
 * @param bitstream bitstream of encoded signal
 * @return length of signal block
 */
auto Decoder::lengthDecoding(std::vector<char>& bitstream) const -> int {

    int segmentlength = bi2de(&bitstream, lengthbits, 0);
    bitstream.erase(bitstream.begin(), bitstream.begin() + lengthbits);

    return segmentlength;
}

/**
 * @brief return sampling rate
 * @return sampling rate
 */
auto Decoder::getFS() const -> int {
    return fs;
}

}  // namespace VC_PWQ
