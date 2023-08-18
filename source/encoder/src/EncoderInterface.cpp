//=======================================================================
/** @file EncoderInterface.cpp
 *  @author Andreas Noll, Lars Nockenberg
 *
 * This file is part of the 'VC-PWQ' library
 *
 * This class can be used to easily encode specific files or all files in a subfolder.
 *
 * (c) 2023. This work is licensed under a CC BY-NC 3.0 license.
 *
 */
//=======================================================================

#include "../include/EncoderInterface.hpp"

namespace VC_PWQ {

/**
 * @brief constructor
 * @param fs_new sampling frequency, only needed for txt files
 */
EncoderInterface::EncoderInterface(int fs_new) : fs(fs_new) {}

/**
 * @brief encode all signals in a folder using the multichannel codec and puts it in defined folder
 * @details if the output folder does not exist, it is generated
 * @param inFolder folder of the input signals
 * @param outFolder folder of the encoded signals
 * @param bl block length
 * @param bitbudget bit budget for the encoder
 * @param appendix appendix to the file name
 * @param maxChannels maximum channel number
 * @return status (-1 for failed, 0 for success)
 */
auto EncoderInterface::encodeFolderMD(const std::string& inFolder,
                                      const std::string& outFolder,
                                      int bl,
                                      int bitbudget,
                                      std::string appendix,
                                      int maxChannels) const -> int {

    std::string bin = ".binary";
    std::string wav = ".wav";
    std::string txt = ".txt";
    std::string csv = ".csv";
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

        if (filename.find(wav) != std::string::npos || filename.find(txt) != std::string::npos ||
            filename.find(csv) != std::string::npos) {

            std::string productname = filename;
            productname.erase(0, inFolder.size() + 1);  // folder
            productname.insert(0, prefix);
            auto pos_appendix = (long)productname.find('.');
            productname.erase(productname.begin() + pos_appendix, productname.end());
            productname.append(appendix.begin(), appendix.end());
            productname.append(bin.begin(), bin.end());
            std::vector<std::vector<double>> sig_rec;
            std::cout << "input filename: " << filename << std::endl;
            std::cout << "output filename: " << productname << std::endl;
            encodeFileMD(filename, productname, bl, bitbudget, maxChannels);
        }
    }
    return 0;
}

/**
 * @brief encode all signals in a folder using the single channel codec and puts it in defined folder
 * @details if the output folder does not exist, it is generated
 * @param inFolder folder of the input signals
 * @param outFolder folder of the encoded signals
 * @param bl blocklength
 * @param bitbudget bitbudget for the encoder
 * @param appendix appendix to the file name
 * @return status (-1 for failed, 0 for success)
 */
auto EncoderInterface::encodeFolder1D(const std::string& inFolder,
                                      const std::string& outFolder,
                                      int bl,
                                      int bitbudget,
                                      std::string appendix) const -> int {
    std::string bin = ".binary";
    std::string wav = ".wav";
    std::string txt = ".txt";
    std::string csv = ".csv";
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

        if (filename.find(wav) != std::string::npos || filename.find(txt) != std::string::npos ||
            filename.find(csv) != std::string::npos) {

            std::string productname = filename;
            productname.erase(0, inFolder.size() + 1);  // folder
            productname.insert(0, prefix);
            auto pos_appendix = (long)productname.find('.');
            productname.erase(productname.begin() + pos_appendix, productname.end());
            productname.append(appendix.begin(), appendix.end());
            productname.append(bin.begin(), bin.end());
            std::vector<std::vector<double>> sig_rec;
            encodeFile1D(filename, productname, bl, bitbudget);
            std::cout << "input filename: " << filename << std::endl;
            std::cout << "output filename: " << productname << std::endl;
        }
    }
    return 0;
}

/**
 * @brief encode a multichannel signal using the single channel codec (extended to multichannel)
 * @param inFile filename of the input signal
 * @param outFile filename of the input signal
 * @param bl blocklength
 * @param bitbudget bitbudget for the encoder
 * @param maxChannels maximum number of channels
 * @return status (-1 for failed, 0 for success)
 */
auto EncoderInterface::encodeFileMD(const std::string& inFile,
                                    const std::string& outFile,
                                    int bl,
                                    int bitbudget,
                                    int maxChannels) const -> int {
    std::vector<std::vector<double>> buffer;
    int fs = 0;
    if (inFile.find(".wav") != std::string::npos) {
        AudioFile<double> file(inFile);
        buffer = file.samples;
        fs = (int)file.getSampleRate();
    } else {
        readTXTMatrix(buffer, inFile);
        if (this->fs == 0) {
            std::cout << "please specify a sampling frequency for .txt files" << std::endl;
            return -1;
        }
        fs = this->fs;
    }

    Encoder encoder(bl, fs, maxChannels);

    std::vector<char> bitstream = encoder.encodeMD(buffer, bitbudget);

    saveAsBinary(outFile, bitstream);

    return 0;
}

/**
 * @brief encode a single channel signal
 * @param inFile filename of the input signal
 * @param outFile filename of the input signal
 * @param bl blocklength
 * @param bitbudget bitbudget for the encoder
 * @return status (-1 for failed, 0 for success)
 */
auto EncoderInterface::encodeFile1D(const std::string& inFile, const std::string& outFile, int bl, int bitbudget) const
    -> int {
    std::vector<double> buffer;
    int fs = 0;
    size_t channels = 0;
    if (inFile.find(".wav") != std::string::npos) {
        AudioFile<double> file(inFile);
        buffer = file.samples.at(0);
        fs = (int)file.getSampleRate();
        channels = file.getNumChannels();
    } else {
        std::vector<std::vector<double>> buffer_txt;
        readTXTMatrix(buffer_txt, inFile);
        buffer = buffer_txt.at(0);
        channels = buffer_txt.size();
        if (this->fs == 0) {
            std::cout << "please specify a sampling frequency for .txt files" << std::endl;
            return -1;
        }
        fs = this->fs;
    }
    if (channels > 1) {
        std::cout << "File contains more than one channel. Only first channel will be encoded" << std::endl;
        std::cout << channels << std::endl;
    }

    Encoder encoder(bl, fs);

    std::vector<char> bitstream = encoder.encode1D(buffer, bitbudget);

    saveAsBinary(outFile, bitstream);

    return 0;
}

}  // namespace VC_PWQ
