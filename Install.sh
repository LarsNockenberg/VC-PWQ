#!/usr/bin/env bash

FFTW_VERSION="3.3.10"

curl https://www.fftw.org/fftw-"$FFTW_VERSION".tar.gz --output fftw-"$FFTW_VERSION".tar.gz
tar -xzf fftw-"$FFTW_VERSION".tar.gz

pushd fftw-"$FFTW_VERSION" || exit
./configure
make
make install
popd || exit

rm -rf fftw-"$FFTW_VERSION" fftw-"$FFTW_VERSION".tar.gz
