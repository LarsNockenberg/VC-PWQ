//=======================================================================
/** @file ArithDec.cpp
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

#include "../include/ArithDec.hpp"

namespace VC_PWQ {

/**
 * @brief constructor
 */
ArithDec::ArithDec() {
    resetCounter();
}

/**
 * @brief initialize arithmetic decoder
 * @details the arithmetic decoding is performed for one bit at a time, because the context in SPIHT is only known for
 * the very next bit; this function is called at the beginning of each signal block decoding process
 * @param instream input bitstream
 * @param pos position of first relevant bit
 * @param length length of bistream belonging to the current signal block
 */
void ArithDec::initDecoding(std::vector<char>* instream, size_t pos, size_t length) {
    this->instream = instream;
    in_index = pos;
    max_index = pos + length - 1;

    // get first 10 digits
    in_leading = 0;
    int shift = SHIFT;
    for (int i = 0; i < DIGITS_START; i++) {
        if (i < length) {
            in_leading += (int)(instream->at(in_index)) << shift;
            in_index++;
            shift--;
        } else {
            break;
        }
    }

    range_diff = RANGE_MAX;
    range_lower = 0;
    range_upper = RANGE_MAX;
}

/**
 * @brief decode a single bit
 * @param context context number for the current bit
 */
auto ArithDec::decode(int context) -> int {

    double p = round((double)(counter.at(context)) / (double)(counter_total.at(context)) *
                     RANGE_MAX);  // p scaled to full range
    int compare = (int)((double)(range_diff)*p) / RANGE_MAX;

    // if p is close to 0 or maximum, value has to be adjusted
    if (compare == 0) {
        compare = 1;
    } else if (compare == range_diff) {
        compare = range_diff - 1;
    }

    int value = in_leading - range_lower;

    // determine decoded symbol; range is updated
    int s = 0;
    if (value < compare) {
        s = 0;
        range_upper = range_lower + compare;
    } else {
        s = 1;
        range_lower = range_lower + compare;
    }

    // check, if range has to be adjusted
    while (true) {

        if (range_upper <= HALF) {
            range_lower = range_lower << 1;
            range_upper = range_upper << 1;
            if (in_index <= max_index) {
                in_leading = (in_leading << 1) + instream->at(in_index);
                in_index++;
            } else {
                in_leading = in_leading << 1;
            }
        } else if (range_lower >= HALF) {
            range_lower = (range_lower - HALF) << 1;
            range_upper = (range_upper - HALF) << 1;
            if (in_index <= max_index) {
                in_leading = ((in_leading - HALF) << 1) + instream->at(in_index);
                in_index++;
            } else {
                in_leading = (in_leading - HALF) << 1;
            }
        } else if (range_lower >= FIRST_QTR && range_upper <= THIRD_QTR) {
            range_lower = (range_lower - FIRST_QTR) << 1;
            range_upper = (range_upper - FIRST_QTR) << 1;
            if (in_index <= max_index) {
                in_leading = ((in_leading - FIRST_QTR) << 1) + instream->at(in_index);
                in_index++;
            } else {
                in_leading = (in_leading - FIRST_QTR) << 1;
            }
        } else {
            break;
        }
    }

    range_diff = range_upper - range_lower;

    // update counter for probabilities
    if (s == 0) {
        counter.at(context)++;
    }
    counter_total.at(context)++;

    return s;
}

/**
 * @brief reset context counter
 */
void ArithDec::resetCounter() {
    for (int i = 0; i < CONTEXTS; i++) {
        counter.at(i) = RESET / 2;
        counter_total.at(i) = RESET;
    }
}

/**
 * @brief rescale context counter
 */
void ArithDec::rescaleCounter() {
    for (int i = 0; i < CONTEXTS; i++) {
        counter.at(i) = (int)((double)counter.at(i) / (double)counter_total.at(i) * RESIZE);
        if (counter.at(i) == 0) {
            counter.at(i) = 1;
        }
        counter_total.at(i) = RESIZE;
    }
}

}  // namespace VC_PWQ
