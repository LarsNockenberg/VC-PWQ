//=======================================================================
/** @file ArithEnc.cpp
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

#include "../include/ArithEnc.hpp"

using namespace VC_PWQ;

/**
 * @brief constructor
 */
ArithEnc::ArithEnc() {
    resetCounter();
}

/**
 * @brief arithmetic encoder
 * @param instream input bitstream
 * @param context context bitstream
 * @param outstream output bitstream
 */
void ArithEnc::encode(std::vector<char>* instream, std::vector<int>* context, std::vector<char>* outstream) {

    // init loop variables
    int range_lower = 0;
    int range_upper = RANGE_MAX;
    int bits_to_follow = 0;
    int range_diff = 0;
    int new_symbol = 0;
    int c = 0;
    int range_add = 0;

    for (int i = 0; i < instream->size(); i++) {

        // calculate range
        range_diff = range_upper - range_lower;
        new_symbol = (int)(unsigned char)instream->at(i);

        c = context->at(i);

        double p = round((double)counter.at(c) / (double)counter_total.at(c) * RANGE_MAX);  // p scaled to full range
        range_add = ((int)((double)range_diff * p)) / RANGE_MAX;

        // if p is close to 0 or maximum, value has to be adjusted
        if (range_add == 0) {
            range_add = 1;
        } else if (range_add == range_diff) {
            range_add = range_diff - 1;
        }

        if (new_symbol == 0) {
            range_upper = range_lower + range_add;
        } else {
            range_lower = range_lower + range_add;
        }

        // adjust range to prevent underflow and set output
        while (true) {

            if (range_upper <= HALF) {
                if (bits_to_follow > 0) {
                    outstream->push_back(0);
                    for (int j = 0; j < bits_to_follow; j++) {
                        outstream->push_back(1);
                    }
                    bits_to_follow = 0;
                } else {
                    outstream->push_back(0);
                }
            } else if (range_lower >= HALF) {
                if (bits_to_follow > 0) {
                    outstream->push_back(1);
                    for (int j = 0; j < bits_to_follow; j++) {
                        outstream->push_back(0);
                    }
                    bits_to_follow = 0;
                } else {
                    outstream->push_back(1);
                }
                range_lower -= HALF;
                range_upper -= HALF;
            } else if (range_lower >= FIRST_QTR && range_upper <= THIRD_QTR) {
                bits_to_follow++;
                range_lower -= FIRST_QTR;
                range_upper -= FIRST_QTR;
            } else {
                break;
            }
            range_lower = range_lower << 1;
            range_upper = range_upper << 1;
        }

        // update counter for probabilities
        if (instream->at(i) == 0) {
            counter.at(c)++;
        }
        counter_total.at(c)++;
    }

    // set remainder to output
    if (bits_to_follow > 0) {
        // if bits_to_follow is not reset to 0, setting the LSB of the output to 1
        // is the shortest encoded number in the correct range
        outstream->push_back(1);
    } else {
        int val = HALF;
        while (range_lower > 0) {
            if (val < range_upper) {
                outstream->push_back(1);
                range_lower -= val;
                range_upper -= val;
            } else {
                outstream->push_back(0);
            }
            val = val >> 1;
        }
    }

    // cut off unnecessary zeros at end
    size_t index_end = outstream->size() - 1;
    while (outstream->at(index_end) == 0 && index_end >= 0) {
        index_end--;
    }
    outstream->resize(index_end + 1);
}

/**
 * @brief reset the context counter for probability estimation
 */
void ArithEnc::resetCounter() {
    for (int i = 0; i < CONTEXTS; i++) {
        counter.at(i) = RESET / 2;
        counter_total.at(i) = RESET;
    }
}

/**
 * @brief rescale the context counter, so new data has more impact on the probability
 */
void ArithEnc::rescaleCounter() {
    for (int i = 0; i < CONTEXTS; i++) {
        counter.at(i) = (int)((double)counter.at(i) / (double)counter_total.at(i) * RESIZE);
        if (counter.at(i) == 0) {
            counter.at(i) = 1;
        }
        counter_total.at(i) = RESIZE;
    }
}
