#!/bin/bash
set -e

. $(dirname $0)/cmake.sh

main() {
    # Clone and build FEBio Studio 
    git clone --depth 1 -b febiostudio3 https://github.com/febiosoftware/FEBioStudio.git
    ln -s febio-sdk FEBioStudio/febio-sdk
	pushd FEBioStudio
    ./ci/Linux/build.sh
    popd
    
	run_cmake
	pushd cmbuild
	make -j $(nproc)
	popd
}

main
