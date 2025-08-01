call "%VS2019INSTALLDIR%\VC\Auxiliary\Build\vcvars64.bat"

:: Clone and build FEBio Studio 
git clone --depth 1 -b febiostudio3 https://github.com/febiosoftware/FEBioStudio.git
set Qt_Root="c:/usr/local/Qt/6.7.3/msvc2019_64"
:: TODO: Cmake requires 6 runs to generate correctly
for /l %%a in (1, 1, 6) do (
cmake -L . -B cmbuild ^
  -DQt_Root=%Qt_Root% ^
  -DQt6Widgets_DIR="%Qt_Root%\lib\cmake\Qt6Widgets" ^
  -DFEBio_SDK=..\febio4-sdk ^
  -DWINDEPLOYQT_EXECUTABLE="%Qt_Root%\bin\windeployqt.exe" ^
  -DUSE_FFMPEG=ON ^
  -DUSE_TETGEN=ON ^
  -DUSE_MMG=ON ^
  -DUSE_SSH=ON ^
  -DUSE_SSL=ON ^
  -DCAD_FEATURES=ON ^
  -DUSE_NETGEN=ON ^
  -DMODEL_REPO=ON ^
  -DUSE_ITK=ON ^
  -DBUILD_UPDATER=ON ^
  -DUSE_PYTHON=ON ^
  -DPython3_INCLUDE_DIR="C:\Program Files\Python313\include" ^
  -DPython3_LIBRARY="C:\Program Files\Python313\libs\python313.lib" ^
  -DPython3_EXECUTABLE="C:\Program Files\Python313\python.exe"
)
cd cmbuild
msbuild /v:m /P:Configuration=Release  /clp:ErrorsOnly /m:%NUMBER_OF_PROCESSORS% ALL_BUILD.vcxproj
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
