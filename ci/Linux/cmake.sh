QT_DIR="/opt/Qt/6.7.3/gcc_64"
FBS_SDK="./FEBioStudio/"
run_cmake() {
	cmake -L . -B cmbuild \
		-DQt_Root=$QT_DIR \
		-DFEBio_SDK=febio4-sdk \
        -DFBS_SDK=$FBS_SDK 
}
