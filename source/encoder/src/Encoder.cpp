//=======================================================================
/** @file Encoder.cpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * This class is the centerpiece of the VC-PWQ encoder.
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#include "../include/Encoder.hpp"

namespace VC_PWQ {

/**
 * @brief constructor of the encoder
 * @param bl_new block length
 * @param fs_new sampling frequency; only a fixed number of values supported
 * @param maxChannels specify maximum number of channels supported; default on 8
 */
Encoder::Encoder(int bl_new, int fs_new, int maxChannels)
    : bl(bl_new), fs(fs_new), channelbits(ceil(log2(maxChannels + 1))) {

    switch (bl) {
        case BL_4:
            lengthbits = LENGTHBITS_4;
            break;
        case BL_3:
            lengthbits = LENGTHBITS_3;
            break;
        case BL_2:
            lengthbits = LENGTHBITS_2;
            break;
        case BL_1:
            lengthbits = LENGTHBITS_1;
            break;
        case BL_0:
            lengthbits = LENGTHBITS_0;
            break;
        default:
            lengthbits = LENGTHBITS_4;
            break;
    }

    dwtlevel = (int)log2(bl) - 2;

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

    pm.init(bl, fs);
}

/**
 * @brief encode a signal with multiple channels using the VC-PWQ for each channel individually
 * @details the signal will be padded to full blocks of length bl and if the bitstream is not empty, the generated bits
 * will be appended
 * @param sig input signal
 * @param bitbudget    limit for bitallocation
 * @return encoded bitstream
 */
auto Encoder::encodeMD(std::vector<std::vector<double>>& sig, int bitbudget) -> std::vector<char> {

    if (bitbudget > MAX_BITS * l_book) {
        std::cerr << "bit budget too high, switching to maximum" << std::endl;
        bitbudget = MAX_BITS * l_book;
    }
    std::vector<char> bitstream;

    int channels = (int)sig.size();
    arithmetic.resetCounter();

    std::vector<double> temp = sig.at(0);
    size_t length = temp.size();
    auto numblocks = (size_t)ceil((double)length / (double)bl);
    bitstream.reserve(BINARY_RESERVE * numblocks * channels);

    size_t diff = (numblocks * bl) - length;
    if (diff > 0) {
        for (int c = 0; c < channels; c++) {
            sig.at(c).resize(((size_t)numblocks * (size_t)bl), 0);
        }
    }

    if (encodeChannels(channels, &bitstream) == -1) {
        return bitstream;
    }

    fsEncode(&bitstream);
    for (size_t b = 0; b < numblocks; b++) {

        std::vector<std::vector<double>> waveletsMD;
        waveletsMD.reserve(channels);
        std::vector<std::vector<double>> SMR_MD;
        SMR_MD.reserve(channels);
        std::vector<std::vector<double>> bandenergy_MD;
        bandenergy_MD.reserve(channels);
        // std::vector<double*> t;
        // t.reserve(channels);
        for (int c = 0; c < channels; c++) {
            temp = sig.at(c);
            // t.push_back(temp->data() + (b * bl));

            std::vector<double> buffer_in(bl, 0);
            std::copy(temp.begin() + (long)(b * bl), temp.begin() + (long)(b + 1) * bl, buffer_in.begin());
            std::vector<double> buffer_out = DWT(buffer_in, dwtlevel);
            waveletsMD.push_back(buffer_out);

            pmResult pmres = pm.getSMR(buffer_in);
            SMR_MD.push_back(pmres.SMR);
            bandenergy_MD.push_back(pmres.bandenergy);
        }

        for (int c = 0; c < channels; c++) {
            headerEncoding(&bitstream);
            encodeBlock(waveletsMD[c], SMR_MD[c], bandenergy_MD[c], bitstream, bitbudget);
        }
    }
    return bitstream;
}

/**
 * @brief encode an signal with a single channel using the VC-PWQ
 * @details the signal will be padded to full blocks of length bl and if the bitstream is not empty, the generated bits
 * will be appended
 * @param sig input signal
 * @param bitbudget    limit for bitallocation
 * @return encoded bitstream
 */
auto Encoder::encode1D(std::vector<double>& sig, int bitbudget) -> std::vector<char> {

    if (bitbudget > MAX_BITS * l_book) {
        std::cerr << "bit budget too high, switching to maximum" << std::endl;
        bitbudget = MAX_BITS * l_book;
    }

    std::vector<char> bitstream;

    arithmetic.resetCounter();

    fsEncode(&bitstream);

    size_t length = sig.size();
    auto numblocks = (size_t)ceil((double)length / (double)bl);
    bitstream.reserve(numblocks * BINARY_RESERVE);

    size_t diff = (numblocks * bl) - length;
    if (diff > 0) {
        sig.resize((numblocks * bl), 0);
    }
    for (size_t b = 0; b < numblocks; b++) {
        headerEncoding(&bitstream);

        std::vector<double> buffer_in(bl, 0);
        std::copy(sig.begin() + (long)(b * bl), sig.begin() + (long)((b + 1) * bl), buffer_in.begin());
        std::vector<double> wavelets = DWT(buffer_in, dwtlevel);

        pmResult pmres = pm.getSMR(buffer_in);
        encodeBlock(wavelets, pmres.SMR, pmres.bandenergy, bitstream, bitbudget);
    }

    return bitstream;
}

/**
 * @brief encode a signal block
 * @details the signal will be padded to full blocks of length bl and if the bitstream is not empty, the generated bits
 * will be appended
 * @param block_dwt input signal block
 * @param SMR signal-to-mask ratio
 * @param bandenergy bandenergy
 * @param bitstream    bitstream to write to
 * @param bitbudget    limit for bitallocation
 * @return quantized signal block
 */
auto Encoder::encodeBlock(std::vector<double>& block_dwt,
                          std::vector<double> SMR,
                          std::vector<double> bandenergy,
                          std::vector<char>& bitstream,
                          int bitbudget) -> std::vector<double> {

    std::vector<double> block_dwt_quant(bl, 0);
    std::vector<int> block_intquant(bl, 0);
    // double *sig_pointer = psig;
    std::vector<double> SNR(l_book, 0);
    std::vector<double> MNR(l_book, 0);
    std::vector<double> noiseenergy(l_book, 0);
    std::vector<int> bitalloc(l_book, 0);
    int bitalloc_sum = 0;

    // if the signal contains only zeros
    if (checkZeros(block_dwt, bl)) {
        std::vector<char> blockstream;
        lengthEncoding(bitstream, blockstream);
    } else {
        double qwavmax = 0;
        std::vector<char> bitwavmax;
        bitwavmax.reserve(WAVMAXLENGTH);
        maximumWaveletCoefficient(block_dwt, &qwavmax, &bitwavmax);

        // Quantization
        bitalloc_sum = 0;
        for (int i = 0; i < bl; i++) {
            block_dwt_quant[i] = 0;
        }

        int i = 0;
        for (int block = 0; block < l_book; block++) {
            for (; i < book_cumulative[block + 1]; i++) {
                noiseenergy[block] += pow(block_dwt[i] - block_dwt_quant[i], 2);
            }
        }

        while (bitalloc_sum < bitbudget) {
            updateNoise(bandenergy, noiseenergy, SNR, MNR, SMR);
            for (int i = 0; i < l_book; i++) {
                if (bitalloc[i] >= MAX_BITS) {
                    MNR[i] = INFINITY;
                }
            }
            int index = findMinInd(MNR);
            if (bitalloc_sum - bitalloc[l_book - 1] >= MAX_BITS * dwtlevel) {
                int temp = bitalloc[l_book - 1];
                bitalloc[l_book - 1] = bitbudget - MAX_BITS * dwtlevel;
                bitalloc_sum += bitalloc[l_book - 1] - temp;
            } else {
                bitalloc[index]++;
                bitalloc_sum++;
            }

            uniformQuant(block_dwt, block_dwt_quant, book_cumulative[index], book[index], qwavmax, bitalloc[index]);

            noiseenergy[index] = 0;
            int i = book_cumulative[index];
            for (; i < book_cumulative[index + 1]; i++) {
                noiseenergy[index] += pow(block_dwt[i] - block_dwt_quant[i], 2);
            }
        }

        // scale signal to int values
        int bitmax = findMax(bitalloc);
        int intmax = 1 << bitmax;
        double multiplicator = (double)intmax / (double)qwavmax;
        for (int i = 0; i < bl; i++) {
            block_intquant[i] = (int)round((block_dwt_quant[i] * multiplicator));
        }
        losslessEncoding(block_intquant, bitwavmax, bitmax, bitstream);
    }
    return block_dwt_quant;
}

/**
 * @brief lossless encoding of a signal block
 * @param block_intquant input signal block
 * @param bitwavmax maximum amplitude
 * @param bitmax maximum allocated bits
 * @param bitstream    bitstream to write to
 */
void Encoder::losslessEncoding(std::vector<int>& block_intquant,
                               std::vector<char>& bitwavmax,
                               int bitmax,
                               std::vector<char>& bitstream) {
    std::vector<char> SPIHT_stream;
    SPIHT_stream.reserve(BINARY_RESERVE);
    std::vector<int> SPIHT_context;
    SPIHT_context.reserve(BINARY_RESERVE);
    spiht.encode(block_intquant, dwtlevel, &bitwavmax, bitmax, SPIHT_stream, SPIHT_context);

    std::vector<char> arithmetic_stream;
    arithmetic_stream.reserve(BINARY_RESERVE);
    arithmetic.encode(&SPIHT_stream, &SPIHT_context, &arithmetic_stream);
    arithmetic.rescaleCounter();

    lengthEncoding(bitstream, arithmetic_stream);
    bitstream.insert(bitstream.end(), arithmetic_stream.begin(), arithmetic_stream.end());
}

/**
 * @brief encode sampling frequency
 * @details only discrete values are possible; change for concrete application (decoder accordingly, too)
 * @param bitstream bitstream to write to
 */
void Encoder::fsEncode(std::vector<char>* bitstream) const {

    if (fs == FS_0) {
        bitstream->push_back(0);
        bitstream->push_back(0);
    } else if (fs == FS_1) {
        bitstream->push_back(0);
        bitstream->push_back(1);
    } else if (fs == FS_2) {
        bitstream->push_back(1);
        bitstream->push_back(0);
    } else {
        bitstream->push_back(1);
        bitstream->push_back(1);
    }
}

/**
 * @brief encode channel count
 * @param channels number of channels
 * @param bitstream bitstream to write to
 * @return status (0 if success, -1 if too many channels in signal)
 */
auto Encoder::encodeChannels(int channels, std::vector<char>* bitstream) const -> int {
    if (channels > ((int)(pow(2, channelbits) - 1))) {
        std::cout << "too many channels; adjust maxChannels at constructor" << std::endl;
        return -1;
    }
    de2bi(channels, bitstream, channelbits);
    return 0;
}

/**
 * @brief encode blocklength
 * @param bitstream bitstream to write to
 */
void Encoder::headerEncoding(std::vector<char>* bitstream) const {

    switch (bl) {
        case BL_0:
            bitstream->push_back(1);
            break;

        case BL_1:
            bitstream->push_back(0);
            bitstream->push_back(1);
            break;

        case BL_2:
            bitstream->push_back(0);
            bitstream->push_back(0);
            bitstream->push_back(1);
            break;

        case BL_3:
            bitstream->push_back(0);
            bitstream->push_back(0);
            bitstream->push_back(0);
            bitstream->push_back(0);
            break;

        case BL_4:
            bitstream->push_back(0);
            bitstream->push_back(0);
            bitstream->push_back(0);
            bitstream->push_back(1);
            break;

        default:
            break;
    }
}

/**
 * @brief encode length of block bitstream
 * @param outstream bitstream to write to
 * @param blockstream stream of signal block
 */
void Encoder::lengthEncoding(std::vector<char>& outstream, std::vector<char>& blockstream) const {
    int segmentlength = (int)blockstream.size();
    int max_size = pow(2, lengthbits) - 1;
    if (segmentlength > max_size) {
        blockstream.resize(max_size);
        segmentlength = max_size;
    }
    de2bi(segmentlength, &outstream, lengthbits);
}

/**
 * @brief find maximum wavelet coefficient and encode it
 * @param sig input signal vector
 * @param qwavmax   pointer for returning maximum wavelet coefficient
 * @param bitwavmax bitstream vector for encoding
 */
void Encoder::maximumWaveletCoefficient(std::vector<double>& sig, double* qwavmax, std::vector<char>* bitwavmax) {

    double wavmax = findMax(sig);

    int integerpart = 0;
    int integerbits = 0;
    int fractionbits = 0;
    char mode = 0;
    if (wavmax < 1) {
        integerbits = INTEGERPART_0;
        fractionbits = FRACTIONPART_0;
        mode = 0;
    } else {
        integerpart = 1;
        integerbits = INTEGERPART_1;
        fractionbits = FRACTIONPART_1;
        mode = 1;
    }

    *qwavmax = maxQuant(wavmax - (double)integerpart, integerbits, fractionbits) + integerpart;
    bitwavmax->push_back(mode);
    bitwavmax->resize(WAVMAXLENGTH);
    de2bi((int)((*qwavmax - (double)integerpart) * pow(2, (double)fractionbits)),
          bitwavmax,
          integerbits + fractionbits,
          1);
}

/**
 * @brief update quantization noise in wavelet bands
 * @details beside noiseenergy, also SNR an MNR will be updated respectively
 * @param bandenergy energy of the original signal
 * @param noiseenergy quantization noise
 * @param SNR Signal-to-Noise-Ratio
 * @param MNR Mask-to-Noise-Ratio
 * @param SMR Signal-to-Mask-Ratio
 */
void Encoder::updateNoise(std::vector<double>& bandenergy,
                          std::vector<double>& noiseenergy,
                          std::vector<double>& SNR,
                          std::vector<double>& MNR,
                          std::vector<double>& SMR) const {
    for (int i = 0; i < l_book; i++) {
        SNR[i] = 10 * log10(bandenergy[i] / noiseenergy[i]);
        MNR[i] = SNR[i] - SMR[i];
    }
}

}  // namespace VC_PWQ