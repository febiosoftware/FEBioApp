QT_ROOT="$HOME/local/x86_64/QtNew/6.7.3/macos"
FBS_SDK="./FEBioStudio/"
run_cmake() {
	cmake -L . -B cmbuild \
		-DQt_Root=$QT_ROOT \
		-DFEBio_SDK=febio4-sdk \
        -DFBS_SDK=$FBS_SDK \
		-DHOMEBREW=ON \
        -DUSE_MKL_OMP=ON \
        -DMKL_OMP=/opt/intel/oneapi/compiler/latest/mac/compiler/lib/libiomp5.dylib \
        -DOMP_INC=/Users/gitRunner/local/x86_64/homebrew/opt/libomp/include
}


