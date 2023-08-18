//=======================================================================
/** @file main.cpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * The main method in this file can be taken as an example to compress .wav or .txt files contained in a subfolder.
 *
 * The codec uses the FFTW-library, which has to be installed beforehand. It can be obtained by running install.sh.
 *
 * The codec currently only supports sampling frequencies of 8000, 2800 and 2500 Hz. For other sampling frequencies, the
 * codec will work correctly, but the decoded .wav file will have a sampling frequency of 0 Hz. To account for that, the
 * correct sampling frequency can be specified for the constructor of EncoderInterface.
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#include "../../decoder/include/DecoderInterface.hpp"
#include "../../encoder/include/EncoderInterface.hpp"

using VC_PWQ::DecoderInterface;
using VC_PWQ::EncoderInterface;

auto main(int argc, const char* argv[]) -> int {

    const auto args = std::vector<const char*>(argv, argv + argc);
    std::vector<std::string> arguments;
    arguments.reserve(args.size());
    for (const auto& a : args) {
        arguments.emplace_back(a);
    }

    std::string folder_orig = "data_original";
    std::string folder_enc = "data_compressed";
    std::string folder_dec = "data_decoded";

    int maxchannels = 8;
    int budget = 120;
    int bl = 512;
    int fs = 2800;  // needed for .txt files as input or custom sampling frequencies; codec currently supports
                    // 2500, 2800 and 8000 Hz

    bool enable_md = false;

    for (size_t i = 0; i < arguments.size(); i++) {
        const auto l = arguments[i];
        if (l == "-i") {
            i++;
            folder_orig = arguments[i];
        } else if (l == "-c") {
            i++;
            folder_enc = arguments[i];
        } else if (l == "-o") {
            i++;
            folder_dec = arguments[i];
        } else if (l == "-b") {
            i++;
            budget = std::stoi(arguments[i]);
        } else if (l == "-md") {
            enable_md = true;
        } else if (l == "-bl") {
            i++;
            bl = std::stoi(arguments[i]);
        } else if (l == "-fs") {
            i++;
            fs = std::stoi(arguments[i]);
        } else if (l == "-ch") {
            i++;
            maxchannels = std::stoi(arguments[i]);
        } else if (l == "-h" || l == "--help") {
            std::cout << "This is the demo program of the VC-PWQ. It can be used to compress vibrotactile signals "
                         "provided as .wav, .txt and .csv files (channels as rows) in a folder."
                      << std::endl;
            std::cout << "The following arguments can be passed to the codec:" << std::endl;
            std::cout << "-i <folder>: \t\tspecify input folder. Default: 'data_original'" << std::endl;
            std::cout << "-c <folder>: \t\tspecify compressed output folder. Default: 'data_compressed'" << std::endl;
            std::cout << "-o <folder>: \t\tspecify decoded output folder. Default: 'data_decoded'" << std::endl;
            std::cout << "-md: \t\t\tenable multichannel mode. Default: disabled" << std::endl;
            std::cout << "-bl <integer number>: \tspecify blocklength. Has to be a power of 2 and between 32 and 512. "
                         "Default: 512"
                      << std::endl;
            std::cout
                << "-b <integer number>: \tspecify bit budget between 1 and 15*(log2(blocklength)-1). Default: 120"
                << std::endl;
            std::cout << "-fs <integer number>: \tspecify sampling frequency. Default: 2800" << std::endl;
            std::cout << "-ch <folder>: \t\tspecify maximum channel number. Default: 8" << std::endl;
            std::cout << "-h/--help: \t\tdisplay this help text" << std::endl;
            return 0;
        }
    }

    std::cout << "input folder: " << folder_orig << std::endl;
    std::cout << "compressed folder: " << folder_enc << std::endl;
    std::cout << "output folder: " << folder_dec << std::endl;

    const std::vector<int> bitbudgets = {budget};  // can be extended to a range of different budgets

    bool txt_mode = false;

    EncoderInterface encInterface(fs);            // fs can be left out for .wav files - encoder takes fs from .wav file
    DecoderInterface decInterface(txt_mode, fs);  // fs optional, for .wav files with custom sampling frequencies

    std::cout << "starting encoding" << std::endl;
    for (const auto& b : bitbudgets) {

        // Encode .wav files in folder folder_orig with the specified settings and put it into folder folder_enc
        if (enable_md) {
            encInterface.encodeFolderMD(folder_orig, folder_enc, bl, b, "_" + std::to_string((int)b), maxchannels);
        } else {
            encInterface.encodeFolder1D(folder_orig, folder_enc, bl, b, "_" + std::to_string((int)b));
        }
    }

    std::cout << "encoding done" << std::endl;

    std::cout << "starting decoding" << std::endl;
    // Encode .binary files in folder "Data_compressed" and put it into folder "Data_decoded"
    if (enable_md) {
        decInterface.decodeFolderMD(folder_enc, folder_dec, maxchannels);
    } else {
        decInterface.decodeFolder1D(folder_enc, folder_dec);
    }

    std::cout << "decoding done" << std::endl;

    return 0;
}
