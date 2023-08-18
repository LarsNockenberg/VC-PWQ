//=======================================================================
/** @file DecoderInterface.cpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * This class can be used to easily decode specific files or all files in a subfolder.
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#include "../include/DecoderInterface.hpp"

namespace VC_PWQ {

/**
 * @brief constructor
 * @param txt_mode_new set to true, if decoded file should be saved as .txt instead of .wav
 * @param fs_new sampling frequency, only needed for custom sampling frequencies and correct saving of .wav
 * @param delimiter_new optional delimiter for .txt saving
 */
DecoderInterface::DecoderInterface(bool txt_mode_new, int fs_new, std::string delimiter_new)
    : txt_mode(txt_mode_new), fs(fs_new), delimiter(delimiter_new) {}

/**
 * @brief decode all signals in a folder using the single channel codec (extended to multichannel signals) and puts it
 * in defined folder
 * @details if the output folder does not exist, it is generated
 * @param inFolder folder of the encoded signals
 * @param outFolder folder of the decoded signals
 * @param maxChannels maximum channel count expected in signals
 * @return status (-1 if failed, 0 if success)
 */
auto DecoderInterface::decodeFolderMD(const std::string& inFolder, const std::string& outFolder, int maxChannels) const
    -> int {
    std::string bin = ".binary";
    std::string type;
    if (txt_mode) {
        type = ".txt";
    } else {
        type = ".wav";
    }
    if (!std::filesystem::is_directory(inFolder)) {
        std::cout << "folder not found: " << inFolder << std::endl;
        return -1;
    }
    std::filesystem::path path(inFolder);
    std::filesystem::path path_products(outFolder);
    if (path.empty()) {
        std::cout << "no data found" << std::endl;
        return -1;
    }
    if (!is_directory(path_products)) {
        create_directory(path_products);
    }

    std::string prefix = outFolder + "/";  // save folder
    for (const auto& entry : std::filesystem::directory_iterator(inFolder)) {
        std::string filename = entry.path();

        if (filename.find(bin) != std::string::npos) {
            std::string productname = filename;
            productname.erase(0, inFolder.size() + 1);  // folder
            productname.insert(0, prefix);
            productname.erase(productname.end() - (long)bin.size(), productname.end());
            productname.append(type.begin(), type.end());
            std::vector<std::vector<double>> sig_rec;
            decodeFileMD(filename, sig_rec, productname, maxChannels);
            std::cout << "input filename: " << filename << std::endl;
            std::cout << "output filename: " << productname << std::endl;
        }
    }
    return 0;
}

/**
 * @brief decode all signals in a folder using the single channel codec and puts it in defined folder
 * @details if the output folder does not exist, it is generated
 * @param inFolder folder of the encoded signals
 * @param outFolder folder of the decoded signals
 * @return status (-1 if failed, 0 if success)
 */
auto DecoderInterface::decodeFolder1D(const std::string& inFolder, const std::string& outFolder) const -> int {
    std::string bin = ".binary";
    std::string type;
    if (txt_mode) {
        type = ".txt";
    } else {
        type = ".wav";
    }
    std::filesystem::path path(inFolder);
    std::filesystem::path path_products(outFolder);
    if (path.empty()) {
        std::cout << "no data found" << std::endl;
        return -1;
    }
    if (!is_directory(path_products)) {
        create_directory(path_products);
    }

    std::string prefix = outFolder + "/";  // save folder

    for (const auto& entry : std::filesystem::directory_iterator(inFolder)) {
        std::string filename = entry.path();

        if (filename.find(bin) != std::string::npos) {
            std::string productname = filename;
            productname.erase(0, inFolder.size() + 1);  // folder
            productname.insert(0, prefix);
            productname.erase(productname.end() - (long)bin.size(), productname.end());
            productname.append(type.begin(), type.end());
            std::vector<double> sig_rec;
            decodeFile1D(filename, sig_rec, productname);
            std::cout << "input filename: " << filename << std::endl;
            std::cout << "output filename: " << productname << std::endl;
        }
    }
    return 0;
}

/**
 * @brief decode specific multichannel signal using the single channel codec (extended to multichannel signals) and puts
 * it in defined folder
 * @details if the output folder does not exist, it is generated
 * @param inFile input file name
 * @param outFile output file name
 * @param maxChannels maximum expected channel count
 * @return sampling frequency
 */
auto DecoderInterface::decodeFileMD(const std::string& inFile,
                                    std::vector<std::vector<double>>& sig_rec,
                                    const std::string& outFile,
                                    int maxChannels) const -> int {
    std::vector<char> bitstream;
    loadBinary(inFile, bitstream);

    Decoder decoder(maxChannels);
    sig_rec = decoder.decodeMD(bitstream);
    double fs_dec = decoder.getFS();

    if (!(outFile.empty())) {
        if (txt_mode) {
            saveMatrixScientific(sig_rec, outFile, ",");
        } else {
            if (fs_dec == 0) {
                fs_dec = fs;
            }
            AudioFile<double> out;
            out.setSampleRate((int)fs_dec);
            out.setAudioBuffer(sig_rec);
            out.save(outFile);
        }
    }

    return fs;
}

/**
 * @brief decode specific single channel signal using the single channel codec and puts it in defined folder
 * @details if the output folder does not exist, it is generated
 * @param inFile input file name
 * @param sig_rec decoded signal
 * @param outFile output file name
 * @return sampling frequency
 */
auto DecoderInterface::decodeFile1D(const std::string& inFile,
                                    std::vector<double>& sig_rec,
                                    const std::string& outFile) const -> int {
    std::vector<char> bitstream;
    loadBinary(inFile, bitstream);

    Decoder decoder;
    sig_rec = decoder.decode1D(bitstream);
    int fs_dec = decoder.getFS();

    if (!(outFile.empty())) {
        std::vector<std::vector<double>> buffer;
        buffer.push_back(sig_rec);
        if (txt_mode) {
            saveMatrixScientific(buffer, outFile, delimiter);
        } else {
            if (fs_dec == 0) {
                fs_dec = fs;
            }
            AudioFile<double> out;
            out.setSampleRate((int)fs_dec);
            out.setAudioBuffer(buffer);
            out.save(outFile);
        }
    }

    return fs;
}

}  // namespace VC_PWQ
