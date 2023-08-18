//=======================================================================
/** @file EncoderInterface.hpp
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

#ifndef EncoderInterface_hpp
#define EncoderInterface_hpp

#include <filesystem>

#include <AudioFile.h>

#include "Encoder.hpp"

namespace VC_PWQ {

static constexpr size_t BUFFERSIZE_INTERFACE = 10000;

class EncoderInterface {
  public:
    EncoderInterface(int fs = 0);

    auto encodeFolderMD(const std::string& inFolder,
                        const std::string& outFolder,
                        int bl,
                        int bitbudget,
                        std::string appendix = "",
                        int maxChannels = MAXCHANNELS_DEFAULT) const -> int;
    auto encodeFolder1D(const std::string& inFolder,
                        const std::string& outFolder,
                        int bl,
                        int bitbudget,
                        std::string appendix = "") const -> int;
    auto encodeFileMD(const std::string& inFile,
                      const std::string& outFile,
                      int bl,
                      int bitbudget,
                      int maxChannels = MAXCHANNELS_DEFAULT) const -> int;
    auto encodeFile1D(const std::string& inFile, const std::string& outFile, int bl, int bitbudget) const -> int;

  protected:
    int fs;
};

}  // namespace VC_PWQ

#endif /* EncoderInterface_hpp */
