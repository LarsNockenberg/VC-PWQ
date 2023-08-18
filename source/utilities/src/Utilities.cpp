//=======================================================================
/** @file Utilities.cpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * A collection of the utility functions needed for the VC-PWQ
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#include "../include/Utilities.hpp"

/**
 * @brief perform uniform quantization
 * @param in input signal
 * @param out quantized output signal
 * @param length length of the signal array
 * @param max maximum value for quantization
 * @param bits bit depth
 */
void VC_PWQ::uniformQuant(std::vector<double>& in,
                          std::vector<double>& out,
                          int start,
                          int length,
                          double max,
                          int bits) {
    double delta = max / (1 << bits);
    double max_q = delta * ((1 << bits) - 1);
    for (int i = start; i < start + length; i++) {
        double sign = sgn(in[i]);
        double q = sign * delta * floor(std::abs(in[i]) / delta + HALF_QUANT);
        if (std::abs(q) > max_q) {
            out[i] = sign * max_q;
        } else {
            out[i] = q;
        }
    }
}

auto VC_PWQ::uniformQuant(double& in, double max, int bits) -> double {
    double out = 0;
    double delta = max / (1 << bits);
    double max_q = delta * ((1 << bits) - 1);
    double sign = sgn(in);
    double q = sign * delta * floor(std::abs(in) / delta + HALF_QUANT);
    if (std::abs(q) > max_q) {
        out = sign * max_q;
    } else {
        out = q;
    }
    return out;
}

/**
 * @brief perform quantization for maximum wavelet coefficient
 * @details quantization uses adaptive number of bits for integer and fraction part
 * @param in input number
 * @param b1 integerbits
 * @param b2 fractionbits
 */
auto VC_PWQ::maxQuant(double in, int b1, int b2) -> double {

    double max = ((double)(1 << (b1 + b2)) - 1) / (1 << b2);

    double q = in;
    if (q >= max) {
        // NOLINTNEXTLINE[cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers]
        q = sgn(q) * max * 0.999;
    }
    double delta = pow(2, (double)-b2);
    return ceil(std::abs(q) / delta) * delta;
}

/**
 * @brief return sign of a value
 * @param val input value
 * @return sign of the input value
 */
auto VC_PWQ::sgn(int val) -> int {
    return (int)(0 < val) - (int)(val < 0);
}

/**
 * @brief return sign of a value
 * @param val input value
 * @return sign of the input value
 */
auto VC_PWQ::sgn(double val) -> double {
    return (double)(0 < val) - (double)(val < 0);
}

/**
 * @brief return maximum value in vector
 * @param data input vector
 * @return maximum value of input vector
 */
auto VC_PWQ::findMaxVector(std::vector<double>& data) -> double {
    double max = data.at(0);

    for (size_t i = 1; i < data.size(); i++) {
        if (data.at(i) > max) {
            max = data.at(i);
        }
    }

    return max;
}

/**
 * @brief return absolute maximum value in array
 * @param data input array
 * @return absolute maximum value of input array
 */
auto VC_PWQ::findMax(std::vector<double>& data) -> double {
    double max = 0;

    for (const auto& d : data) {
        double temp = std::abs(d);
        if (temp > max) {
            max = temp;
        }
    }

    return max;
}

/**
 * @brief return absolute maximum value in array
 * @param data input array
 * @return absolute maximum value of input array
 */
auto VC_PWQ::findMax(std::vector<int>& data) -> int {
    int max = 0;

    for (const auto& d : data) {
        int temp = std::abs(d);
        if (temp > max) {
            max = temp;
        }
    }

    return max;
}

auto VC_PWQ::findMinInd(std::vector<double>& data) -> int {
    double min = data[0];
    int index = 0;
    for (int i = 0; i < data.size(); i++) {
        if (data[i] < min) {
            min = data[i];
            index = i;
        }
    }

    return index;
}

auto VC_PWQ::findMinInd(std::vector<std::vector<double>>& data) -> std::pair<int, int> {
    double min = data[0][0];
    int min_i = 0;
    int min_j = 0;
    for (int i = 0; i < data.size(); i++) {
        auto temp = data[i];
        for (int j = 0; j < temp.size(); j++) {
            if (temp[j] < min) {
                min = temp[j];
                min_i = i;
                min_j = j;
            }
        }
    }
    return std::pair<int, int>({min_i, min_j});
}

/**
 * @brief return maximum of two values
 * @param v1 first value
 * @param v2 second value
 * @return maximum value of input values
 */
auto VC_PWQ::Max(double v1, double v2) -> double {
    if (v1 > v2) {
        return v1;
    }
    return v2;
}

auto VC_PWQ::mean(const std::vector<double>& data) -> double {
    double mean_val = 0;
    for (const auto& d : data) {
        mean_val += d;
    }
    mean_val = mean_val / data.size();
    return mean_val;
}

void VC_PWQ::abs_vector(std::vector<double>& data) {
    for (auto& d : data) {
        d = std::abs(d);
    }
}

auto VC_PWQ::variance(const std::vector<double>& in) -> double {
    double var = 0;
    double mean_val = mean(in);
    for (const auto& i : in) {
        var += pow(i - mean_val, 2);
    }
    var = var / in.size();
    return var;
}

auto VC_PWQ::energy(const std::vector<double>& in) -> double {
    double e = 0;
    for (const auto& i : in) {
        e += pow(i, 2);
    }
    return e;
}

/**
 * @brief convert decimal value (int) to binary number and add to bitstream
 * @param val input value
 * @param outstream bitstream to write to
 * @param length number of bits of binary number
 */
void VC_PWQ::de2bi(int val, std::vector<char>* outstream, int length) {
    if (val > (int)pow(2, (double)(length)) - 1) {
        std::cout << "de2bi: value too large for bit size, value: " << val << ", bits: " << length
                  << ", max: " << (int)pow(2, (double)(length)) - 1 << std::endl;
    }
    int n = length;
    while (n > 0) {
        outstream->push_back(val % 2);
        val = val >> 1;
        n--;
    }
}

/**
 * @brief convert decimal value (int) to binary number and add to bitstream at specified position
 * @param val input value
 * @param outstream bitstream to write to
 * @param length number of bits of binary number
 * @param pos starting index in outstream
 */
void VC_PWQ::de2bi(int val, std::vector<char>* outstream, int length, size_t pos) {
    int n = length;
    while (n > 0) {
        outstream->at(pos) = val % 2;
        val = val >> 1;
        n--;
        pos++;
    }
}

/**
 * @brief convert binary number at specified position in stream to decimal number
 * @param pointer array of binary number
 * @param length number of bits of binary number
 * @param pos starting index in vector
 * @return decimal number (int)
 */
auto VC_PWQ::bi2de(std::vector<char>* pointer, int length, size_t pos) -> int {
    int val = 0;
    for (int i = 0; i < length; i++) {
        val += pointer->at(pos + i) << i;
    }
    return val;
}

/**
 * @brief convert binary number at specified position in stream to decimal number
 * @param pointer array of binary number
 * @param length number of bits of binary number
 * @param pos starting index in vector
 * @return decimal number (int)
 */
auto VC_PWQ::bi2de(std::vector<int>& data) -> int {
    int val = 0;
    int i = 0;
    for (const auto& v : data) {
        val += v << i;
        i++;
    }
    return val;
}

template <typename T>
void VC_PWQ::saveVector(std::vector<T>* pointer, const std::string& name) {
    std::ofstream outFile(name, std::ofstream::out);
    if (outFile.is_open()) {
        for (int i = 0; i < pointer->size() - 1; i++) {
            outFile << (int)pointer->at(i) << ", ";
        }
        outFile << (int)pointer->at(pointer->size() - 1);
        outFile.close();
    } else {
        std::cout << "failed to write file" << std::endl;
    }
}

void VC_PWQ::saveVectorScientific(const std::vector<double>& data, const std::string& name) {
    std::ofstream outFile(name, std::ofstream::out);
    std::scientific(outFile);
    if (outFile.is_open()) {
        for (int i = 0; i < data.size() - 1; i++) {
            outFile << data[i] << ", ";
        }
        outFile << data[data.size() - 1];
        outFile.close();
    } else {
        std::cout << "failed to write file" << std::endl;
    }
}

void VC_PWQ::saveMatrixScientific(const std::vector<std::vector<double>>& data,
                                  const std::string& name,
                                  const std::string& delimiter) {
    std::ofstream outFile(name, std::ofstream::out);
    std::scientific(outFile);
    if (outFile.is_open()) {
        for (auto d : data) {
            for (auto it = d.begin(); it < d.end() - 1; ++it) {
                outFile << (double)*it << delimiter;
            }
            outFile << (double)*d.end() << std::endl;
        }
        outFile.close();
    } else {
        std::cout << "failed to write file" << std::endl;
    }
}

auto VC_PWQ::readTXTMatrix(std::vector<std::vector<double>>& buffer, const std::string& name) -> int {
    if (!std::filesystem::is_regular_file(std::filesystem::path(name))) {
        return 0;
    }
    std::ifstream inFile;
    inFile.open(name, std::ifstream::in);
    std::string line;
    buffer.clear();
    while (getline(inFile, line))  // for multiple rows
    {
        std::string delimiter;
        if (line.find(',') != std::string::npos) {
            delimiter = ",";
        } else if (line.find('\t') != std::string::npos) {
            delimiter = "\t";
        } else {
            delimiter = " ";
        }
        std::vector<double> buffer_line;
        while (line.compare(" ") == 1) {
            line = line.substr(1, line.size() - 1);
        }
        size_t pos = 0;
        pos = line.find(delimiter);
        while (pos <= line.size()) {
            std::string substring = line.substr(0, pos);

            std::istringstream os(substring);
            double d = 0;
            os >> d;  // scientific notation as input works
            buffer_line.push_back(d);

            line = line.substr(pos + 1, line.size() - 1);
            while (line.compare(" ") == 1) {
                line = line.substr(1, line.size() - 1);
            }

            pos = line.find(delimiter);
        }

        std::istringstream os(line);
        double d = 0;
        os >> d;  // scientific notation as input works
        buffer_line.push_back(d);
        buffer.push_back(buffer_line);
    }
    if (buffer.size() > buffer[0].size()) {
        std::cout << "transposing input data" << std::endl;
        buffer = transposeMatrix(buffer);
    }
    return 1;
}

/**
 * @brief get bit of decimal number at specified position
 * @param in input number
 * @param bit position of desired bit
 * @return bit at specified position
 */
auto VC_PWQ::bitget(int in, int bit) -> int {
    int mask = 1 << (bit - 1);

    if ((in & mask) > 0) {
        return 1;
    }
    return 0;
}

/**
 * @brief check if the signal only contains zeros
 * @param sig input signal
 * @param length length of the input signal
 * @return boolean is true if the signal only contains zeros
 */
auto VC_PWQ::checkZeros(std::vector<double>& sig, int length) -> bool {
    bool zero = true;
    for (int i = 0; i < length; i++) {
        if (std::abs(sig[i]) > 1e-10) {
            zero = false;
        }
    }
    return zero;
}

/**
 * @brief load binary file
 * @param name filename
 * @param bitstream buffer to write to
 */
void VC_PWQ::loadBinary(const std::string& name, std::vector<char>& bitstream) {
    std::ifstream infile(name, std::ifstream::binary);
    if (infile.is_open()) {
        infile.seekg(0, infile.end);
        size_t size = infile.tellg();
        infile.seekg(0);
        bitstream.reserve(size);
        for (size_t i = 0; i < size; i++) {
            char data;
            infile.read(&data, 1);
            auto data_long = (unsigned long)data;
            std::bitset<BYTE_SIZE> data_bits(data_long);
            for (int i = 0; i < BYTE_SIZE; i++) {
                bitstream.push_back((char)data_bits[i]);
            }
        }
        infile.close();
    }
}

/**
 * @brief save binary file
 * @param name filename
 * @param bitstream buffer to get data from
 */
void VC_PWQ::saveAsBinary(const std::string& name, std::vector<char>& bitstream) {
    size_t j = 0;
    size_t out_length = bitstream.size();
    std::ofstream outfile(name, std::ofstream::binary);
    bool go_on = true;
    while (go_on) {
        std::bitset<BYTE_SIZE> bits;
        for (int i = 0; i < BYTE_SIZE; i++) {
            bits[i] = bitstream.at(j);
            j++;
            if (j >= out_length) {
                go_on = false;
                break;
            }
        }
        unsigned long bits_long = bits.to_ulong();
        char bits_char = static_cast<char>(bits_long);
        outfile.write(&bits_char, 1);
    }
    outfile.close();
}

auto VC_PWQ::transposeMatrix(const std::vector<std::vector<double>>& in) -> std::vector<std::vector<double>> {
    std::vector<std::vector<double>> out;
    out.reserve(in[0].size());
    for (int i = 0; i < in[0].size(); i++) {
        std::vector<double> temp;
        temp.reserve(in.size());
        for (int j = 0; j < in.size(); j++) {
            temp.push_back(in[j][i]);
        }
        out.push_back(temp);
    }
    return out;
}