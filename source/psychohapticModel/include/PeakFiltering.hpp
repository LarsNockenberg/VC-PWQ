//=======================================================================
/** @file PeakFiltering.hpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * The PeakFiltering methods detect peaks in a spectrum.
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#ifndef PeakFiltering_hpp
#define PeakFiltering_hpp

#include <cmath>
#include <complex>
#include <vector>

#include "../../utilities/include/Utilities.hpp"

namespace VC_PWQ::PeakFiltering {

struct peak {
    size_t location;
    double height;
};

static constexpr double PEAK_HUGE_VAL = 2147483647;  // 2^32 - 1

auto FindAllPeakLocations(std::vector<double>& x) -> std::vector<peak>;
auto PeakProminence(std::vector<double>& spectrum, std::vector<peak>& peaks) -> std::vector<peak>;
auto FilterPeakCriterion(std::vector<peak>& input, double min_peak_val) -> std::vector<peak>;
auto FindPeaks(std::vector<double>& spectrum, double min_peak_prominence, double min_peak_height) -> std::vector<peak>;

}  // namespace VC_PWQ::PeakFiltering

#endif /* PeakFiltering_hpp */
