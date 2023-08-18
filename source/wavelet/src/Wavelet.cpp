//=======================================================================
/** @file Wavelet.cpp
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

#include "../include/Wavelet.hpp"

namespace VC_PWQ {

void filter(std::vector<double> in, std::vector<double>& out, double h) {
    for (auto& v : in) {
        v = v * h;
    }
    std::transform(
        in.begin(), in.end(), out.begin(), out.begin(), [](double add, double v) -> double { return v + add; });
    std::transform(in.begin(), in.end() - 1, out.begin() + 1, out.begin() + 1, [](double add, double v) -> double {
        return v + add;
    });
    out[0] = out[0] + in[0];
}

void filter_shift(std::vector<double> in, std::vector<double>& out, double h) {
    for (auto& v : in) {
        v = v * h;
    }
    std::transform(
        in.begin() + 1, in.end(), out.begin(), out.begin(), [](double add, double v) -> double { return v + add; });
    size_t end = in.size() - 1;
    out[end] = out[end] + in[end];
    std::transform(in.begin() + 1, in.end(), out.begin() + 1, out.begin() + 1, [](double add, double v) -> double {
        return v + add;
    });
    out[0] = out[0] + in[0];
}

auto DWT(std::vector<double> in, int level) -> std::vector<double> {

    auto n = (int)in.size();

    for (int k = 1; k <= level; k++) {

        std::vector<double> X0;
        std::vector<double> X1;
        int n_half = n / 2;
        X0.reserve(n_half);
        X1.reserve(n_half);
        for (auto it = in.begin(); it < in.begin() + n; it += 2) {
            X0.push_back(*it);
        }
        int i = 0;
        for (auto it = in.begin() + 1; it < in.begin() + n; it += 2) {
            X1.push_back(*it);
        }

        filter_shift(X0, X1, h1);
        filter(X1, X0, h2);
        filter_shift(X0, X1, h3);
        filter(X1, X0, h4);

        std::transform(X0.begin(), X0.end(), in.begin(), [](double v) -> double { return v * scaleFactor; });
        std::transform(X1.begin(), X1.end(), in.begin() + n_half, [](double v) -> double { return -v / scaleFactor; });

        n = n_half;
    }
    return in;
}

auto inv_DWT(std::vector<double> in, int level) -> std::vector<double> {

    auto n = (int)((double)in.size() * (double)pow(2, 1 - level));

    for (int k = 1; k <= level; k++) {

        int n_half = n / 2;
        std::vector<double> X0(n_half, 0);
        std::vector<double> X1(n_half, 0);

        std::transform(in.begin(), in.begin() + n_half, X0.begin(), [](double v) -> double { return v / scaleFactor; });
        std::transform(
            in.begin() + n_half, in.begin() + n, X1.begin(), [](double v) -> double { return -v * scaleFactor; });

        filter(X1, X0, -h4);
        filter_shift(X0, X1, -h3);
        filter(X1, X0, -h2);
        filter_shift(X0, X1, -h1);

        auto it_0 = X0.begin();
        for (auto it = in.begin(); it < in.begin() + n; it += 2) {
            *it = *it_0;
            it_0++;
        }
        auto it_1 = X1.begin();
        for (auto it = in.begin() + 1; it < in.begin() + n; it += 2) {
            *it = *it_1;
            it_1++;
        }

        n = n * 2;
    }
    return in;
}

}  // namespace VC_PWQ
