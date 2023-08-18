//=======================================================================
/** @file PsychohapticModel.cpp
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

#include "../include/PsychohapticModel.hpp"

namespace VC_PWQ {

/**
 * @brief constructor
 */
PsychohapticModel::PsychohapticModel() {}

/**
 * @brief initialize model
 * @param bl block length
 * @param fs sampling frequency
 */
void PsychohapticModel::init(int bl, int fs) {
    this->bl = bl;
    this->fs = fs;

    int dwtlevel = (int)log2((double)bl) - 2;

    l_book = dwtlevel + 1;
    book.resize(l_book);
    book[0] = bl >> dwtlevel;
    book[1] = book[0];
    book_cumulative.resize(l_book + 1);
    book_cumulative[0] = 0;
    book_cumulative[1] = book[0];
    book_cumulative[2] = book[1] << 1;
    for (int i = 2; i < l_book; i++) {
        book[i] = book[i - 1] << 1;
        book_cumulative[i + 1] = book_cumulative[i] << 1;
    }

    setFreqVector(fs, bl);
    perceptualThreshold();
}

/**
 * @brief apply psychohaptic model on signal block
 * @details return arrays have to be as large as the book for the DWT
 * @param block input signal
 * @param SMR    return array for SMR
 * @param bandenergy    return array for bandenergy
 */
auto PsychohapticModel::getSMR(std::vector<double>& block) -> pmResult {

    /*std::vector<double> spect;
    spect.reserve(bl);

    fftw_complex* in;
    fftw_complex* out;
    in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * bl * 2);
    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * bl * 2);

    for (int i = 0; i < bl; i++) {
        in[i][0] = block[i];
        in[i][1] = 0;
    }
    for (int i = bl; i < 2 * bl; i++) {
        in[i][0] = 0;
        in[i][1] = 0;
    }

    fftw_plan p = fftw_plan_dft_1d(bl * 2, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute_dft(p, in, out);

    double temp = 1 / sqrt(bl);
    for (int i = 0; i < bl; i++) {
        spect.push_back(FACTOR_LOG_2 * log10(temp * sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1])));
    }

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);*/
    std::vector<double> spect = DCT(block);

    std::vector<double> globalmask(bl, 0);
    globalMaskingThreshold(spect, globalmask);

    pmResult result(l_book);

    std::vector<double> maskenergy(l_book, 0);
    int i = 0;
    for (int b = 0; b < l_book; b++) {
        result.bandenergy[b] = 0;
        for (; i < book_cumulative[b + 1]; i++) {
            result.bandenergy[b] += pow(BASE_LOG, spect[i] / FACTOR_LOG);
            maskenergy[b] += globalmask[i];
        }
        result.SMR[b] = FACTOR_LOG * log10(result.bandenergy[b] / maskenergy[b]);
    }
    return result;
}

/**
 * @brief apply psychohaptic model on signal block, MD
 * @details return arrays have to be as large as the book for the DWT
 * @param block input signal
 * @param SMR    return array for SMR
 * @param bandenergy    return array for bandenergy
 */
void PsychohapticModel::getSMR_MD(std::vector<std::vector<double>>* block,
                                  std::vector<std::vector<double>>& SMR,
                                  std::vector<std::vector<double>>& bandenergy) {
    size_t channels = block->size();

    for (size_t c = 0; c < channels; c++) {
        std::vector<double> temp = block->at(c);
        pmResult pmres = getSMR(temp);
        SMR[c] = pmres.SMR;
        bandenergy[c] = pmres.bandenergy;
    }
}

/**
 * @brief Compute globalmask for a given signal spectrum taking perceptual threshold and peak masking into account
 * @param spect spectrum of signal
 * @param globalmask    return vector for globalmask
 */
void PsychohapticModel::globalMaskingThreshold(std::vector<double>& spect, std::vector<double>& globalmask) {

    double min_peak_height = findMaxVector(spect) - MIN_HEIGHT_DIFF;
    std::vector<peak> peaks = FindPeaks(spect, MIN_PEAK_PROMINENCE, min_peak_height);
    std::vector<double> mask = PeakMask(peaks);
    if (mask.empty()) {
        for (int i = 0; i < bl; i++) {
            globalmask[i] = percthres[i];  // percthres is in linear domain
        }
    } else {
        for (int i = 0; i < bl; i++) {
            globalmask[i] = pow(BASE_LOG, mask.at(i) / FACTOR_LOG) + percthres[i];  // percthres is in linear domain
        }
    }
}

/**
 * @brief Compute signal-independent perceptual threshold curve
 */
void PsychohapticModel::perceptualThreshold() {

    percthres.resize(bl);
    double temp = thr_a / (pow(log10(thr_b), 2));
    percthres[0] = pow(BASE_LOG, (std::abs(temp * pow(log10(thr_c * freqs[0] + thr_b), 2)) - thr_e) / FACTOR_LOG);
    int i = 1;
    while (true) {
        percthres[i] = pow(BASE_LOG, (std::abs(temp * pow(log10(thr_c * freqs[i] + thr_b), 2)) - thr_e) / FACTOR_LOG);
        // limit values at high frequencies
        if (percthres[i] >= 1) {
            percthres[i] = 1;
            break;
        }
        i++;
    }
    i++;
    for (; i < bl; i++) {
        percthres[i] = percthres[i - 1];
    }
}

/**
 * @brief Compute mask based on detected peaks
 * @param peaks_height   height of detected peaks
 * @param peaks_loc   location of detected peaks
 * @param mask   output vector for mask
 */
auto PsychohapticModel::PeakMask(std::vector<peak>& peaks) -> std::vector<double> {

    std::vector<double> mask;
    if (peaks.empty()) {
        mask.clear();
    } else {

        mask.clear();
        mask.reserve(bl);
        double f = freqs[peaks.at(0).location];
        double sum1 = peaks.at(0).height - peak_a + (peak_a / peak_b) * f;
        double factor1 = -peak_c / (f * f);

        for (size_t i = 0; i < bl; ++i) {
            double val = freqs[i];  // freq(1:bl)
            val -= f;               // freq(1:bl)-freq(ploc(i)
            val *= val;             // .^2

            val *= factor1;
            val += sum1;
            mask.push_back(val);
        }
        for (size_t j = 1; j < peaks.size(); j++) {
            f = freqs[peaks.at(j).location];
            sum1 = peaks.at(j).height - peak_a + (peak_a / peak_b) * f;
            factor1 = -peak_c / (f * f);
            for (size_t i = 0; i < bl; ++i) {
                double val = freqs[i];  // freq(1:bl)
                val -= f;               // freq(1:bl)-freq(ploc(i)
                val *= val;             // .^2

                val *= factor1;
                val += sum1;
                if (val > mask.at(i)) {
                    mask.at(i) = val;
                }
            }
        }
    }
    return mask;
}

/**
 * @brief Set frequency vector of Encoder from 0 to fs/2
 * @param fs   sampling rate
 * @param bl   blocklength
 */
void PsychohapticModel::setFreqVector(int fs, size_t bl) {
    freqs.resize(bl);
    double step = ((double)fs) / (double)(2 * bl - 1);
    double freq_cur = 0.0;
    for (size_t i = 0; i < bl; ++i) {
        freqs[i] = freq_cur;
        freq_cur += step;
    }
}

auto PsychohapticModel::DCT(std::vector<double>& data) -> std::vector<double> {

    int size = (int)data.size();
    std::vector<double> spect;
    spect.reserve(size);

    double in[size];
    double out[size];

    std::copy(data.begin(), data.end(), in);

    fftw_plan p = fftw_plan_r2r_1d(size, in, out, FFTW_REDFT10, FFTW_ESTIMATE);
    fftw_execute(p);

    spect.push_back(20 * log10(std::abs(out[0] / (2 * sqrt(size)))));
    double temp = 1 / (sqrt(2 * size));
    for (int i = 1; i < size; i++) {
        // spect.push_back(20 * log10(temp * sqrt(out[0])));
        spect.push_back(20 * log10(std::abs(temp * out[i])));
        // spect.push_back(temp * out[i]);
    }

    fftw_destroy_plan(p);

    return spect;
}

}  // namespace VC_PWQ
