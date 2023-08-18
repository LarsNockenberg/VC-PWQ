//=======================================================================
/** @file Wavelet.hpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * Wavelet transform implementation
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#ifndef Wavelet_hpp
#define Wavelet_hpp

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

namespace VC_PWQ {

static constexpr double h1 = -1.5861343420693648;
static constexpr double h2 = -0.0529801185718856;
static constexpr double h3 = 0.8829110755411875;
static constexpr double h4 = 0.4435068520511142;
static constexpr double scaleFactor = 1.1496043988602418;

void filter(std::vector<double> in, std::vector<double>& out, double h);
void filter_shift(std::vector<double> in, std::vector<double>& out, double h);

auto DWT(std::vector<double> in, int level) -> std::vector<double>;
auto inv_DWT(std::vector<double> in, int level) -> std::vector<double>;

}  // namespace VC_PWQ

#endif /* Wavelet_hpp */
