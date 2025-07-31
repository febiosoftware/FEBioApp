#!/bin/bash
set -e
RUN_POST_BUILD=${RUN_POST_BUILD:=true}

. $(dirname $0)/cmake.sh

main() {
    # Clone and build FEBio Studio 
    git clone --depth 1 -b febiostudio3 https://github.com/febiosoftware/FEBioStudio.git
	pushd FEBioStudio
    ./ci/macOS/build.sh
    popd

	run_cmake
	pushd cmbuild
	make -j $(sysctl -n hw.ncpu)
	popd
}

main
