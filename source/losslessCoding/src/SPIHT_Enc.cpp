//=======================================================================
/** @file SPIHT_Enc.cpp
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

#include "../include/SPIHT_Enc.hpp"

namespace VC_PWQ {

/**
 * @brief constructor
 */
SPIHT_Enc::SPIHT_Enc() {}

/**
 * @brief SPIHT encoder for a 1D signal block
 * @param data quantized values
 * @param level dwt decomposition levels
 * @param bitwavmax already encoded maximum wavelet coefficient
 * @param maxallocbits highest bit depth in quantized signal; defines number of bitplanes in SPIHT
 * @param outstream output bitstream
 * @param context stream of context numbers for arithmetic encoder
 */
void SPIHT_Enc::encode(std::vector<int>& data,
                       int level,
                       std::vector<char>* bitwavmax,
                       int maxallocbits,
                       std::vector<char>& outstream,
                       std::vector<int>& context) {

    // add maxallocbits to stream
    if (maxallocbits > pow(2, MAXALLOCBITS_SIZE) - 1) {
        std::cerr << "SPIHT: too many bits allocated: " << maxallocbits << std::endl;
        maxallocbits = 15;
    }
    de2bi(maxallocbits, &outstream, MAXALLOCBITS_SIZE);
    // add bitwavmax to stream
    outstream.insert(outstream.end(), bitwavmax->begin(), bitwavmax->end());

    // context
    std::vector<int> c(MAXALLOCBITS_SIZE + bitwavmax->size(), CONTEXT_SIDE);
    context.insert(context.end(), c.begin(), c.end());

    // init LIP, LSP, LIS
    int bandsize = 2 << ((int)log2((double)data.size()) - level);
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

    initMaxDescendant(data);

    int n = maxallocbits;
    while (0 <= n) {
        int compare = 1 << n;  // 2^n
        int LSP_idx = (int)LSP.size();
        // sorting pass
        sortingPass(LIP, LSP, LIS, compare, data, outstream, context);

        // refinement pass
        refinementPass(LSP, LSP_idx, data, outstream, context, n);
        n--;
    }
}

void SPIHT_Enc::sortingPass(std::list<int>& LIP,
                            std::list<int>& LSP,
                            std::list<pixel>& LIS,
                            const int compare,
                            std::vector<int>& data,
                            std::vector<char>& outstream,
                            std::vector<int>& context) {

    for (auto it = LIP.begin(); it != LIP.end();) {
        if (std::abs(data[*it]) >= compare) {
            outstream.push_back(1);
            context.push_back(CONTEXT_SIGNIFICANCE_0);
            outstream.push_back((char)(data[*it] >= 0));
            context.push_back(CONTEXT_SIGN);
            LSP.push_back(*it);
            it = LIP.erase(it);
        } else {
            outstream.push_back(0);
            context.push_back(CONTEXT_SIGNIFICANCE_0);
            it++;
        }
    }

    auto it1 = LIS.begin();
    int LISsize = (int)LIS.size();
    for (int i = 0; i < LISsize; i++) {
        // If type A
        if ((*it1).type == 0) {
            int max_d = maxDescendant(*it1);
            if (max_d >= compare) {
                outstream.push_back(1);
                context.push_back(CONTEXT_SIGNIFICANCE_1);
                int y = (*it1).index;
                // Children
                int index = 2 * y;
                if (std::abs(data[index]) >= compare) {
                    LSP.push_back(index);
                    outstream.push_back(1);
                    context.push_back(CONTEXT_SIGNIFICANCE_2);
                    outstream.push_back(data[index] >= 0);
                    context.push_back(CONTEXT_SIGN);
                } else {
                    outstream.push_back(0);
                    context.push_back(CONTEXT_SIGNIFICANCE_2);
                    LIP.push_back(index);
                }

                index = 2 * y + 1;
                if (std::abs(data[index]) >= compare) {
                    LSP.push_back(index);
                    outstream.push_back(1);
                    context.push_back(CONTEXT_SIGNIFICANCE_2);
                    outstream.push_back(data[index] >= 0);
                    context.push_back(CONTEXT_SIGN);
                } else {
                    outstream.push_back(0);
                    context.push_back(CONTEXT_SIGNIFICANCE_2);
                    LIP.push_back(index);
                }

                // Grandchildren
                if ((4 * y + 3) < data.size()) {
                    pixel p = {(*it1).index, 1};
                    LIS.push_back(p);
                    LISsize++;
                }
                it1 = LIS.erase(it1);
            } else {
                outstream.push_back(0);
                context.push_back(CONTEXT_SIGNIFICANCE_1);
                it1++;
            }

            // type B
        } else {
            int max_d = maxDescendant(*it1);
            if (max_d >= compare) {
                outstream.push_back(1);
                context.push_back(CONTEXT_SIGNIFICANCE_3);
                int y = (*it1).index;
                pixel p = {2 * y, 0};
                LIS.push_back(p);
                p = {2 * y + 1, 0};
                LIS.push_back(p);
                LISsize += 2;
                it1 = LIS.erase(it1);
            } else {
                outstream.push_back(0);
                context.push_back(CONTEXT_SIGNIFICANCE_3);
                it1++;
            }
        }
    }
}

void SPIHT_Enc::refinementPass(std::list<int>& LSP,
                               const int LSP_idx,
                               std::vector<int>& data,
                               std::vector<char>& outstream,
                               std::vector<int>& context,
                               const int n) {
    auto it = LSP.begin();
    int temp = 0;
    while (temp < LSP_idx) {

        int s = bitget((int)floor(std::abs(data[*it])), n + 1);
        outstream.push_back(s);
        context.push_back(CONTEXT_REFINEMENT);
        temp++;
        it++;
    }
}

/**
 * @brief return maximum descendant of sample
 * @param j position of sample
 * @param type type of descendant
 */
auto SPIHT_Enc::maxDescendant(pixel p) -> int {
    if (p.type == 1) {
        return maxDescendants1[p.index];
    }
    return maxDescendants[p.index];
}

/**
 * @brief find maximum descendant of all samples; save to array in SPIHT_Enc object
 * @param signal signal to process
 * @param length length of signal
 */
void SPIHT_Enc::initMaxDescendant(std::vector<int>& signal) {

    int start = signal.size() >> 1;

    maxDescendants.resize(start);
    maxDescendants1.resize(start >> 1);

    int p1 = start;
    int p2 = p1 + 1;
    int target = start >> 1;

    for (int i = 0; i < (start >> 1); i++) {
        int v1 = std::abs(signal[p1]);
        int v2 = std::abs(signal[p2]);
        if (v1 > v2) {
            maxDescendants[target] = v1;
        } else {
            maxDescendants[target] = v2;
        }

        p1 += 2;
        p2 += 2;
        target++;
    }

    int width = start >> 1;
    p1 = width;
    p2 = p1 + 1;
    target = width >> 1;

    while (target > 1) {
        for (int i = 0; i < (width >> 1); i++) {
            int v1 = maxDescendants[p1];
            int v2 = maxDescendants[p2];
            if (v1 > v2) {
                maxDescendants1[target] = v1;
            } else {
                maxDescendants1[target] = v2;
            }
            v1 = std::abs(signal[p1]);
            if (v1 > maxDescendants1[target]) {
                maxDescendants[target] = v1;
            } else {
                maxDescendants[target] = maxDescendants1[target];
            }
            v2 = std::abs(signal[p2]);
            if (v2 > maxDescendants[target]) {
                maxDescendants[target] = v2;
            }
            p1 += 2;
            p2 += 2;
            target++;
        }
        width = width >> 1;
        p1 = width;
        p2 = p1 + 1;
        target = width >> 1;
    }
}

}  // namespace VC_PWQ
