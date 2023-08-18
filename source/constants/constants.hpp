//=======================================================================
/** @file constants.hpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * A collection of all necessary constants
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace VC_PWQ {

static constexpr int INTEGERPART_0 = 0;
static constexpr int FRACTIONPART_0 = 7;
static constexpr int INTEGERPART_1 = 3;
static constexpr int FRACTIONPART_1 = 4;
static constexpr size_t WAVMAXLENGTH = 8;

static constexpr int MAXCHANNELS_DEFAULT = 8;

static constexpr size_t MAXALLOCBITS_SIZE = 4;
static constexpr char CONTEXT_SIDE = 0;
static constexpr char CONTEXT_SIGN = 1;
static constexpr char CONTEXT_SIGNIFICANCE_0 = 2;
static constexpr char CONTEXT_SIGNIFICANCE_1 = 3;
static constexpr char CONTEXT_SIGNIFICANCE_2 = 4;
static constexpr char CONTEXT_SIGNIFICANCE_3 = 5;
static constexpr char CONTEXT_REFINEMENT = 6;

static constexpr int FS_0 = 8000;
static constexpr int FS_1 = 2800;
static constexpr int FS_2 = 2500;

static constexpr int BL_0 = 32;
static constexpr int BL_1 = 64;
static constexpr int BL_2 = 128;
static constexpr int BL_3 = 256;
static constexpr int BL_4 = 512;

static constexpr int LENGTHBITS_0 = 10;
static constexpr int LENGTHBITS_1 = 11;
static constexpr int LENGTHBITS_2 = 12;
static constexpr int LENGTHBITS_3 = 13;
static constexpr int LENGTHBITS_4 = 14;

static constexpr size_t MAX_BL = 512;

static constexpr int RANGE_MAX = 1024;
static constexpr int HALF = 512;
static constexpr int FIRST_QTR = 256;
static constexpr int THIRD_QTR = 768;
static constexpr size_t CONTEXTS = 7;

static constexpr int RESET = 16;
static constexpr int RESIZE = 32;

}  // namespace VC_PWQ

#endif /* CONSTANTS_hpp */