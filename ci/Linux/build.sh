#!/bin/bash
set -e

. $(dirname $0)/cmake.sh

main() {
    # Clone and build FEBio Studio 
    git clone --depth 1 -b febiostudio3 https://github.com/febiosoftware/FEBioStudio.git
    ln -s febio-sdk FEBioStudio/febio-sdk
	pushd FEBioStudio

    QT_DIR="/opt/Qt/6.7.3/gcc_64"
    cmake -L . -B cmbuild \
        -DQt_Root=$QT_DIR \
        -DFEBio_SDK=febio4-sdk \
        -DUSE_FFMPEG=ON \
        -DUSE_TETGEN=ON \
        -DUSE_MMG=ON \
        -DUSE_SSH=ON \
        -DUSE_SSL=ON \
        -DCAD_FEATURES=ON \
        -DUSE_NETGEN=ON \
        -DUSE_ITK=ON \
        -DUSE_PYTHON=ON \
        -DBUILD_UPDATER=ON

    pushd cmbuild
    make FSCore GLLib GLWLib OGLLib ImageLib CUILib -j $(nproc) 
    popd
    popd
    
	run_cmake
	pushd cmbuild
	make -j $(nproc)
	popd
}

main
