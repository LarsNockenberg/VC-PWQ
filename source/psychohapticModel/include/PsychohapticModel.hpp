//=======================================================================
/** @file PsychohapticModel.hpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * The Psychohaptic Model is used to analyze a signal block perceptually.
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#ifndef PsychohapticModel_hpp
#define PsychohapticModel_hpp

#include <cmath>
#include <complex>
#include <vector>

#include <fftw3.h>

#include "../../utilities/include/Utilities.hpp"
#include "PeakFiltering.hpp"

namespace VC_PWQ {

static constexpr double MIN_PEAK_PROMINENCE = 12;
static constexpr double MIN_HEIGHT_DIFF = 45;

static constexpr double thr_a = 62;
static constexpr double thr_c = (double)1 / (double)550;
static constexpr double thr_b = 1 - (250 * thr_c);
static constexpr double thr_e = 77;

static constexpr double peak_a = 5;
static constexpr double peak_b = 1400;
static constexpr double peak_c = 30;

static constexpr double BASE_LOG = 10;
static constexpr double FACTOR_LOG = 10;
static constexpr double FACTOR_LOG_2 = 20;

static constexpr int MAX_BITS = 15;

using PeakFiltering::FindPeaks;
using PeakFiltering::peak;

struct pmResult {
    pmResult(int size) : SMR(size, 0), bandenergy(size, 0) {}
    std::vector<double> SMR;
    std::vector<double> bandenergy;
};

class PsychohapticModel {

  public:
    PsychohapticModel();

    void init(int bl, int fs);

    auto getSMR(std::vector<double>& block) -> pmResult;
    void getSMR_MD(std::vector<std::vector<double>>* block,
                   std::vector<std::vector<double>>& SMR,
                   std::vector<std::vector<double>>& bandenergy);

    static auto DCT(std::vector<double>& data) -> std::vector<double>;

  private:
    void globalMaskingThreshold(std::vector<double>& spect, std::vector<double>& globalmask);
    void perceptualThreshold();

    auto PeakMask(std::vector<peak>& peaks) -> std::vector<double>;

    void setFreqVector(int fs, size_t bl);

    std::vector<int> book;
    std::vector<int> book_cumulative;
    int l_book;
    int bl;
    int fs;
    std::vector<double> freqs;
    std::vector<double> percthres;
};

}  // namespace VC_PWQ

#endif /* PsychohapticModel_hpp */
