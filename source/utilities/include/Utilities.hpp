//=======================================================================
/** @file Utilities.hpp
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

#ifndef Utilities_hpp
#define Utilities_hpp

#include <bitset>
#include <cmath>
#include <complex>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// #include <fftw3.h>

namespace VC_PWQ {

static constexpr double HALF_QUANT = 0.5;
static constexpr int BYTE_SIZE = 8;

void uniformQuant(std::vector<double>& in, std::vector<double>& out, int start, int length, double max, int bits);
auto uniformQuant(double& in, double max, int bits) -> double;
auto maxQuant(double in, int b1, int b2) -> double;

auto sgn(int val) -> int;
auto sgn(double val) -> double;

auto findMaxVector(std::vector<double>& data) -> double;

// absolute max
// auto findMax(double* data, int length) -> double;
auto findMax(std::vector<double>& data) -> double;
auto findMax(std::vector<int>& data) -> int;
// auto findMax(int* data, int length) -> int;
auto findMinInd(std::vector<double>& data) -> int;
auto findMinInd(std::vector<std::vector<double>>& data) -> std::pair<int, int>;

auto Max(double v1, double v2) -> double;

auto mean(const std::vector<double>& data) -> double;
void abs_vector(std::vector<double>& data);

auto variance(const std::vector<double>& in) -> double;
auto energy(const std::vector<double>& in) -> double;

void de2bi(int val, std::vector<char>* outstream, int length);
void de2bi(int val, std::vector<char>* outstream, int length, size_t pos);
// auto bi2de(int* pointer, int length) -> int;
auto bi2de(std::vector<char>* pointer, int length, size_t pos) -> int;
auto bi2de(std::vector<int>& data) -> int;

template <typename T>
void saveVector(std::vector<T>* pointer, const std::string& name);
void saveVectorScientific(const std::vector<double>& data, const std::string& name);
void saveMatrixScientific(const std::vector<std::vector<double>>& data,
                          const std::string& name,
                          const std::string& delimiter);
auto readTXTMatrix(std::vector<std::vector<double>>& buffer, const std::string& name) -> int;

auto bitget(int in, int bit) -> int;

// auto SNR(double* sig1, double* sig2, size_t size) -> double;

auto checkZeros(std::vector<double>& sig, int length) -> bool;

void loadBinary(const std::string& name, std::vector<char>& bitstream);
void saveAsBinary(const std::string& name, std::vector<char>& bitstream);

auto transposeMatrix(const std::vector<std::vector<double>>& in) -> std::vector<std::vector<double>>;

}  // namespace VC_PWQ

#endif /* Utilities_hpp */
