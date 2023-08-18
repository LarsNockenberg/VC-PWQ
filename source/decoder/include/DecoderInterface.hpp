//=======================================================================
/** @file DecoderInterface.hpp
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

#ifndef DecoderInterface_hpp
#define DecoderInterface_hpp

#include <cstdio>
#include <filesystem>

#include <AudioFile.h>

#include "../../utilities/include/Utilities.hpp"
#include "Decoder.hpp"

static constexpr size_t BUFFERSIZE = 10000;

namespace VC_PWQ {

class DecoderInterface {
  public:
    DecoderInterface(bool txt_mode = false, int fs = 0, std::string delimiter = ",");

    auto decodeFolderMD(const std::string& inFolder,
                        const std::string& outFolder,
                        int maxChannels = MAXCHANNELS_DEFAULT) const -> int;
    auto decodeFolder1D(const std::string& inFolder, const std::string& outFolder) const -> int;
    auto decodeFileMD(const std::string& inFile,
                      std::vector<std::vector<double> >& sig_rec,
                      const std::string& outFile = "",
                      int maxChannels = MAXCHANNELS_DEFAULT) const -> int;
    auto decodeFile1D(const std::string& inFile, std::vector<double>& sig_rec, const std::string& outFile = "") const
        -> int;

  protected:
    bool txt_mode;
    int fs;
    std::string delimiter;
};

}  // namespace VC_PWQ

#endif /* DecoderInterface_hpp */
