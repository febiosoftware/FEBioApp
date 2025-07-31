call "%VS2019INSTALLDIR%\VC\Auxiliary\Build\vcvars64.bat"

:: Clone and build FEBio Studio 
git clone --depth 1 -b febiostudio3 https://github.com/febiosoftware/FEBioStudio.git
cd FEBioStudio
./ci/Windows/build.bat
cd ..

set Qt_Root="c:/usr/local/Qt/6.7.3/msvc2019_64"
:: TODO: Cmake requires 6 runs to generate correctly
for /l %%a in (1, 1, 6) do (
cmake -L . -B cmbuild ^
  -DQt_Root=%Qt_Root% ^
  -DFEBio_SDK=febio4-sdk ^
  -DFBS_SDK=FEBioStudio ^
  -DWINDEPLOYQT_EXECUTABLE="%Qt_Root%\bin\windeployqt.exe" ^
)

cd cmbuild
msbuild /v:m /P:Configuration=Release  /clp:ErrorsOnly /m:%NUMBER_OF_PROCESSORS% ALL_BUILD.vcxproj
cd ..

exit 0
