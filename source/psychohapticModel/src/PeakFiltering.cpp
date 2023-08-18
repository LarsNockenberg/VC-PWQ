//=======================================================================
/** @file PeakFiltering.cpp
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

#include "../include/PeakFiltering.hpp"

namespace VC_PWQ::PeakFiltering {

/**
 * @brief Compute the locations of all peaks in a signal x
 * @details Emulates Matlab "peaks = findpeaks(x)" Matlab returns index of highest value in plateau with exactly equal
 * values; this function returns middle index A peak is defined as one or more samples of equal height, with at least
 * one smaller sample on each side
 * @param x     signal
 * @param height     return vector for peak heights
 */
auto FindAllPeakLocations(std::vector<double>& x) -> std::vector<peak> {
    // No more than half of the samples can be peaks
    std::vector<peak> peaks;
    peaks.reserve(x.size() / 2);
    size_t num_peaks = 0;
    // Last and first sample can't be maxima
    size_t i = 1;
    size_t i_max = x.size() - 1;
    size_t i_plateau = 0;  // Needed for plateaus
    while (i < i_max) {
        // if ((i == 0) || (x.at(i - 1) < x.at(i))) {
        if (x.at(i - 1) < x.at(i)) {
            if (x.at(i + 1) < x.at(i)) {  // Single sample peak
                peak p = {i, x.at(i)};
                peaks.push_back(p);
                ++num_peaks;
            } else if (x.at(i + 1) == x.at(i)) {  // Plateau of some sort
                i_plateau = i + 1;
                while (x.at(i_plateau + 1) == x.at(i)) {
                    ++i_plateau;
                }
                if (x.at(i_plateau + 1) < x.at(i)) {  // Plateau is peak
                    // peak p = {(i + i_plateau) / 2, x.at(i)}; // center index; not matlab way
                    peak p = {i, x.at(i)};  // index of first sample in plateau
                    peaks.push_back(p);
                    ++num_peaks;
                    i = i_plateau;                           // Skip plateau samples
                } else if (x.at(i_plateau + 1) > x.at(i)) {  // Plateau is no peak
                    i = i_plateau;
                }
            }
            // Do nothing if next sample is bigger than current sample
        }
        ++i;
    }
    // Result can have less than x->length/2 peaks
    peaks.resize(num_peaks);
    return peaks;
}

/**
 * @brief Return the topographic prominence in the spectrum of all input peaks
 * @todo Constrain search space with window_length parameter
 *
 * @param spectrum spectrum of signal input
 * @param peaks_height height of already computed peaks
 * @param peaks_location location of already computed peaks
 * @param prominences_height output vector
 * @param prominences_location output vector
 */
auto PeakProminence(std::vector<double>& spectrum, std::vector<peak>& peaks) -> std::vector<peak> {
    std::vector<peak> prominences;
    size_t num_peaks = peaks.size();
    prominences.reserve(num_peaks);
    // init values
    for (size_t i = 0; i < num_peaks; i++) {
        peak p = {peaks.at(i).location, 0};
        prominences.push_back(p);
    }
    // Determine location of local minima to each side of a peak
    std::vector<int> valley_left(num_peaks, 0);  // init with 0 okay?
    std::vector<int> valley_right(num_peaks, 0);
    // valley_right.reserve(num_peaks);
    for (size_t i = 0; i < num_peaks; ++i) {
        // Seek next larger peak or edge of spectrum to the left
        size_t j_min = 0;  // edge is default
        for (int k = (int)i - 1; k >= 0; --k) {
            if (peaks.at(k).height > peaks.at(i).height) {
                j_min = peaks.at(k).location;
                break;
            }
        }
        // Next sample from current peak to the left
        size_t j_max = peaks.at(i).location - 1;
        // Seek smallest sample in between
        // Not necessary to consider plateaus separately
        size_t j = j_max;
        double min_val_left = peaks.at(i).height;
        while ((j >= j_min) && (j <= j_max)) {
            // Edge of spectrum is equivalent to valley
            if (peaks.at(i).location == 0) {
                /** @todo Behavior ok?:
                 * Do not consider edge height in prominence of edge peak. */
                valley_left.at(i) = -1;
                break;
            }
            // Change to next index if minimum or equal (peak is plateau)
            if (spectrum.at(j) <= min_val_left) {
                min_val_left = spectrum.at(j);
                valley_left.at(i) = j;
            }
            --j;  // TODO: Problem when size_t is -1 at end of last loop?
        }

        // Seek next larger peak or edge of spectrum to the right
        j_max = spectrum.size() - 1;  // edge is default
        for (size_t k = i + 1; k < num_peaks; ++k) {
            if (peaks.at(k).height > peaks.at(i).height) {
                j_max = peaks.at(k).location;
                break;
            }
        }
        // Next sample from current peak to the right
        j_min = peaks.at(i).location + 1;
        j = j_min;
        double min_val_right = peaks.at(i).height;
        while ((j >= j_min) && (j <= j_max)) {
            // Special case when "peak" is at the edge of spectrum
            if (peaks.at(i).location == (int32_t)(j_max)) {
                /** @todo Behavior ok?:
                 * Do not consider edge height in prominence of edge peak. */
                valley_right.at(i) = -1;
                break;
            }
            // Change to next index if minimum or equal (peak is plateau)
            if (spectrum.at(j) <= min_val_right) {
                min_val_right = spectrum.at(j);
                valley_right.at(i) = j;
            }
            ++j;
        }
    }
    double valley_left_height_cur = 0;
    double valley_right_height_cur = 0;
    for (size_t i = 0; i < num_peaks; ++i) {
        if (valley_left.at(i) == -1) {
            valley_left_height_cur = -PEAK_HUGE_VAL;
        } else {
            valley_left_height_cur = spectrum.at(valley_left.at(i));
        }
        if (valley_right.at(i) == -1) {
            valley_right_height_cur = -PEAK_HUGE_VAL;
        } else {
            valley_right_height_cur = spectrum.at(valley_right.at(i));
        }
        prominences.at(i).height = peaks.at(i).height - Max(valley_left_height_cur, valley_right_height_cur);
    }
    return prominences;
}

/**
 * @brief Sort out peaks with height below min_peak_val
 * @param input_height  height vector of the peaks, input
 * @param input_location  location vector of the peaks, input
 * @param result_height   height vector of the peaks, output
 * @param result_location   location vector of the peaks, output
 * @param min_peak_val   minimum value for peaks
 */
auto FilterPeakCriterion(std::vector<peak>& input, double min_peak_val) -> std::vector<peak> {
    std::vector<peak> result;
    size_t length = input.size();
    result.reserve(length);
    size_t num_peaks = 0;
    for (size_t i = 0; i < length; ++i) {
        if (input.at(i).height >= min_peak_val) {
            peak p = {input.at(i).location, input.at(i).height};
            result.push_back(p);
            ++num_peaks;
        }
    }
    result.resize(num_peaks);
    return result;
}

/**
 * @brief Compute the locations of all peaks in a signal x and reduce to most prominent ones
 * @param spectrum  signal spectrum
 * @param min_peak_prominence   minimum prominence of detected peaks
 * @param min_peak_height   minimum height of detected peaks
 * @param result_height   output vector for peak height
 * @param result_location   output vector for peak location
 */
auto FindPeaks(std::vector<double>& spectrum, double min_peak_prominence, double min_peak_height) -> std::vector<peak> {

    std::vector<peak> out;

    std::vector<peak> peaks_all = FindAllPeakLocations(spectrum);

    if (peaks_all.empty()) {
        // printf("\nNo peaks were found!\n");
        return peaks_all;
    }
    // TODO Filter peak height before or after peak prominence detection?
    std::vector<peak> peaks_min_h = FilterPeakCriterion(peaks_all, min_peak_height);

    if (peaks_min_h.empty()) {
        // printf("\nNo peaks were found after filtering for minimum height!\n");
        return peaks_min_h;
    }
    std::vector<peak> prominences = PeakProminence(spectrum, peaks_min_h);

    std::vector<peak> peaks_min_prominence = FilterPeakCriterion(prominences, min_peak_prominence);

    // Save heights instead of prominences
    size_t prominences_length = peaks_min_prominence.size();

    out.reserve(prominences_length);
    for (size_t i = 0; i < prominences_length; ++i) {
        peak p = {peaks_min_prominence.at(i).location, spectrum.at(peaks_min_prominence.at(i).location)};
        out.push_back(p);
    }
    return out;
}

}  // namespace VC_PWQ::PeakFiltering
