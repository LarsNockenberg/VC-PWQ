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

#include "../include/SPIHT_Dec.hpp"

namespace VC_PWQ {

/**
 * @brief constructor
 */
SPIHT_Dec::SPIHT_Dec() : arithDec(new ArithDec()) {}

/**
 * @brief decode a 1D signal block encoded with SPIHT and Arithmetic Coder
 * @details arithmetic coding is also performed, on a bit-by-bit basis
 * @param bitstream bitstream to decode
 * @param pos position of first bit in stream
 * @param streamlength length of bitream belonging to one block
 * @param out pointer to decoded signal
 * @param origlength length of decoded block
 * @param level dwt decomposition levels
 * @param wavmax maximum wavelet coefficient; used as scaling factor
 * @param n_real decoded number of bitplanes is saved to this pointer
 */
void SPIHT_Dec::decode(std::vector<char>& bitstream,
                       size_t pos,
                       size_t streamlength,
                       std::vector<int>& out,
                       int origlength,
                       int level,
                       double* wavmax,
                       int* n_real) {

    arithDec->initDecoding(&bitstream, pos, streamlength);

    for (int i = 0; i < origlength; i++) {
        out[i] = 0;
    }

    // get maxallocBits
    std::vector<int> maxallocbitsArray(MAXALLOCBITS_SIZE, 0);
    getBits(maxallocbitsArray, CONTEXT_SIDE);
    int maxallocbits = bi2de(maxallocbitsArray);

    int mode = getBit(CONTEXT_SIDE);
    std::vector<int> wavmaxArray(WAVMAXLENGTH - 1, 0);
    getBits(wavmaxArray, CONTEXT_SIDE);
    int temp = bi2de(wavmaxArray);
    if (mode == 0) {
        *wavmax = (double)temp * pow(2, -FRACTIONPART_0);
    } else {
        *wavmax = (double)temp * pow(2, -FRACTIONPART_1) + 1;
    }
    *n_real = maxallocbits;

    // init LIP, LSP, LIS
    int bandsize = 2 << ((int)log2((double)origlength) - level);
    std::list<int> LIP;
    for (int i = 0; i < bandsize; i++) {
        LIP.push_back(i);
    }
    std::list<pixel> LIS;
    for (int i = (bandsize / 2); i < bandsize; i++) {
        pixel p = {i, 0};
        LIS.push_back(p);
    }
    std::list<int> LSP;

    int n = maxallocbits;
    while (0 <= n) {
        int compare = 1 << n;  // 2^n
        int LSP_idx = (int)LSP.size();
        // sorting pass
        sortingPass(LIP, out, LSP, LIS, compare);

        // refinement pass
        refinementPass(LSP, LSP_idx, compare, out);

        n--;
    }

    arithDec->rescaleCounter();
}

void SPIHT_Dec::sortingPass(std::list<int>& LIP,
                            std::vector<int>& out,
                            std::list<int>& LSP,
                            std::list<pixel>& LIS,
                            int compare) {
    for (auto it = LIP.begin(); it != LIP.end();) {
        if (getBit(CONTEXT_SIGNIFICANCE_0) == 1) {
            if (getBit(CONTEXT_SIGN) == 1) {
                out[*it] = compare;
            } else {
                out[*it] = -compare;
            }
            LSP.push_back(*it);
            it = LIP.erase(it);
        } else {
            it++;
        }
    }

    auto it1 = LIS.begin();
    int LISsize = (int)LIS.size();
    for (int i = 0; i < LISsize; i++) {
        // If type A
        if ((*it1).type == 0) {
            if (getBit(CONTEXT_SIGNIFICANCE_1) == 1) {
                int y = (*it1).index;
                // Children
                int index = 2 * y;
                if (getBit(CONTEXT_SIGNIFICANCE_2) == 1) {
                    LSP.push_back(index);
                    if (getBit(CONTEXT_SIGN) == 1) {
                        out[index] = compare;
                    } else {
                        out[index] = -compare;
                    }
                } else {
                    LIP.push_back(index);
                }

                index = 2 * y + 1;
                if (getBit(CONTEXT_SIGNIFICANCE_2) == 1) {
                    LSP.push_back(index);
                    if (getBit(CONTEXT_SIGN) == 1) {
                        out[index] = compare;
                    } else {
                        out[index] = -compare;
                    }
                } else {
                    LIP.push_back(index);
                }

                // Grandchildren
                if ((4 * y + 3) < out.size()) {
                    pixel p = {(*it1).index, 1};
                    LIS.push_back(p);
                    LISsize++;
                }
                it1 = LIS.erase(it1);
            } else {
                it1++;
            }

            // type B
        } else {
            if (getBit(CONTEXT_SIGNIFICANCE_3) == 1) {
                int y = (*it1).index;
                pixel p = {2 * y, 0};
                LIS.push_back(p);
                p = {2 * y + 1, 0};
                LIS.push_back(p);
                LISsize += 2;
                it1 = LIS.erase(it1);
            } else {
                it1++;
            }
        }
    }
}

void SPIHT_Dec::refinementPass(std::list<int>& LSP, const int LSP_idx, const int compare, std::vector<int>& out) {
    auto it = LSP.begin();
    int temp = 0;
    while (temp < LSP_idx) {
        if (getBit(CONTEXT_REFINEMENT) == 1) {
            out[*it] += sgn(out[*it]) * compare;
        }
        temp++;
        it++;
    }
}

/**
 * @brief interface function to arithmetic decoder; decodes a single bit for SPIHT
 * @param context context number of bit to decode; defined by SPIHT
 */
auto SPIHT_Dec::getBit(int context) -> int {
    int temp = arithDec->decode(context);
    return temp;
}

/**
 * @brief interface function to arithmetic decoder; decodes multiple bits of the same context for SPIHT
 * @param pointer output array
 * @param length length of output array and number of bits to decode
 * @param context context number of bit to decode; defined by SPIHT
 */
void SPIHT_Dec::getBits(std::vector<int>& out, int context) {
    for (auto& o : out) {
        o = arithDec->decode(context);
    }
}

void SPIHT_Dec::resetCounter() {
    arithDec->resetCounter();
}

}  // namespace VC_PWQ
