# VC-PWQ: Vibrotactile Signal Compression based on Perceptual Wavelet Quantization

This is the 'VC-PWQ' library, a C++ implementation of the codec
VC-PWQ: Vibrotactile Signal Compression based on Perceptual Wavelet Quantization.

## Installation

The codec uses the FFTW-library, which has to be installed beforehand. It can be obtained by running install.sh.

A doxygen documentation can be generated using the command "doxygen doxygen.config"

Compilation of the source code is set up for CMake.

## Usage

The main method provided in source/testprogram/main.cpp can be taken as an example to compress .wav or .txt files
contained in a subfolder. The corresponding executable is 'VC_PWQ'.


To embed the codec into a specific application, the classes Encoder and Decoder can also be used directly.

The codec currently only supports sampling frequencies of 8000, 2800 and 2500 Hz. For other sampling frequencies, the
codec will work correctly, but the decoded .wav file will have a sampling frequency of 0 Hz. To account for that, the
correct sampling frequency can be specified for the constructor of EncoderInterface.

## Citation

If you use this work, please cite the paper ([PDF](https://www.researchgate.net/publication/354083396_VC-PWQ_Vibrotactile_Signal_Compression_based_on_Perceptual_Wavelet_Quantization))

A. Noll, L. Nockenberg, B. Gülecyüz, and E. Steinbach, “VC-PWQ: Vibrotactile Signal Compression based on Perceptual Wavelet Quantization,” in 2021 IEEE World Haptics Conference (WHC), 2021, pp. 427–432. doi: 10.1109/WHC49131.2021.9517217.

BibTeX:

```tex

@INPROCEEDINGS{9517217,
  author={Noll, Andreas and Nockenberg, Lars and Gülecyüz, Basak and Steinbach, Eckehard},
  booktitle={2021 IEEE World Haptics Conference (WHC)}, 
  title={VC-PWQ: Vibrotactile Signal Compression based on Perceptual Wavelet Quantization}, 
  year={2021},
  volume={},
  number={},
  pages={427-432},
  doi={10.1109/WHC49131.2021.9517217}
}
```

Please note Patent US 11 113 934 B2 and DE 10 2019 204 527 B4.


authors: Andreas Noll, Lars Nockenberg

(c) 2023. This work is licensed under a CC BY-NC 3.0 license. 