cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(FEBioApp)

# set the name of the executable
set(FBA_BIN_NAME FEBioApp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_FILES_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeFiles)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER Autogen)

# Set a default build type if none was specified
set(default_build_type "Release")
 
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(NOT Qt_Root)
    set(Qt_Root "" CACHE PATH "Path to the root Qt directory (e.g. /opt/Qt)")
endif()

if(NOT Qt6_DIR OR NOT CMAKE_PREFIX_PATH)
    if(WIN32)
        find_path(Qt_TEMP Qt6Config.cmake
            PATHS ${Qt_Root}/ ${Qt_Root}/* ${Qt_Root}/*/* C:/Qt/* C:/Qt*/* $ENV{HOME}/Qt/* $ENV{HOME}/*/Qt/* /Qt/* /Qt*/*
            PATH_SUFFIXES "lib/cmake/Qt6" "msvc2019_64/lib/cmake/Qt6"
            DOC "Qt CMake Directory"
            NO_DEFAULT_PATH)
    elseif(APPLE)
        find_path(Qt_TEMP Qt6Config.cmake
            PATHS ${Qt_Root}/ ${Qt_Root}/* ${Qt_Root}/*/* /opt/Qt/* /opt/Qt*/* /usr/local/Qt/* /usr/local/Qt*/*
              $ENV{HOME}/Qt/* $ENV{HOME}/*/Qt/* /Qt/* /Qt*/*
            PATH_SUFFIXES "lib/cmake/Qt6" "clang_64/lib/cmake/Qt6"
            DOC "Qt6 CMake Directory"
            NO_DEFAULT_PATH)
    else()
        find_path(Qt_TEMP Qt6Config.cmake
            PATHS ${Qt_Root}/ ${Qt_Root}/* ${Qt_Root}/*/* /opt/Qt/* /opt/Qt*/* /usr/local/Qt/* /usr/local/Qt*/* 
                $ENV{HOME}/Qt/* $ENV{HOME}/*/Qt6 $ENV{HOME}/*/Qt/* /Qt/* /Qt*/* 
            PATH_SUFFIXES "gcc_64/lib/cmake/Qt6" "lib/cmake/Qt6"
            DOC "Qt CMake Directory"
            NO_DEFAULT_PATH)
    endif()
    
    if(Qt_TEMP)
        set(Qt6_DIR ${Qt_TEMP} CACHE INTERNAL "Path to Qt6 installation prefix.")

        set(CMAKE_PREFIX_PATH ${Qt_TEMP}/../../.. CACHE INTERNAL "Qt Prefix.")

        unset(Qt_TEMP CACHE)
    else()
        unset(Qt_TEMP CACHE)
    endif()
    
    if(NOT Qt6_DIR OR NOT CMAKE_PREFIX_PATH)
        message(FATAL_ERROR "Unable to locate Qt6Config.cmake. Please check the value of Qt6_Root and set it to the root directory of your Qt6 installation (e.g. /opt/Qt).")
    endif()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

##### Search for library and include directories #####

if(WIN32)
  list(APPEND CMAKE_PREFIX_PATH ${DEPENDENCIES})
endif()

include(FindDependencies.cmake)

#### Check if paths are valid and find libs ####

function(findLib libDir libOut libName)
    # Optional arguments ARGV3-ARVG6 can be used as alernative names for the library
    find_library(TEMP NAMES ${libName} ${ARGV3} ${ARGV4} ${ARGV5} ${ARGV6}
        PATHS ${${libDir}} NO_DEFAULT_PATH)
    
    if(TEMP)
        set(${libOut} ${TEMP} PARENT_SCOPE)
        unset(TEMP CACHE)
    else()
        if(WIN32)
            message(SEND_ERROR "Could not find ${libName}.lib. Check ${libDir}.")
        elseif(APPLE)
            message(SEND_ERROR "Could not find lib${libName}.so, lib${libName}.a, or lib${libName}.dylib Check ${libDir}.")
        else()
            message(SEND_ERROR "Could not find lib${libName}.so, or lib${libName}.a. Check ${libDir}")
        endif()
        unset(TEMP CACHE)
    endif()
endfunction()

# Find FEBio Libs
set(FEBio_LIB_NAMES fecore febiolib febiomech xml febioplot febiofluid febiomix feamr febioopt)

if (WIN32)
    set(FEBio_DEBUG_LIBS "")
    set(FEBio_RELEASE_LIBS "")

    foreach(name IN LISTS FEBio_LIB_NAMES)
        find_library(TEMP NAMES ${name} 
            PATHS ${FEBio_LIB_DIR} PATH_SUFFIXES Debug NO_DEFAULT_PATH)
        list(APPEND FEBio_DEBUG_LIBS ${TEMP})
        unset(TEMP CACHE)
    endforeach(name)

    foreach(name IN LISTS FEBio_LIB_NAMES)
        find_library(TEMP NAMES ${name} 
            PATHS ${FEBio_LIB_DIR} PATH_SUFFIXES Release NO_DEFAULT_PATH)
        list(APPEND FEBio_RELEASE_LIBS ${TEMP})
        unset(TEMP CACHE)
    endforeach(name)

    unset(FEBio_LIB_NAMES)
else()
    set(FEBio_LIBS "")

    foreach(name IN LISTS FEBio_LIB_NAMES)
        findLib(FEBio_LIB_DIR FEBio_LIB ${name})
        list(APPEND FEBio_LIBS ${FEBio_LIB})
        unset(FEBio_LIB)
    endforeach(name)

    unset(FEBio_LIB_NAMES)
endif()

# Find FEBio Studio Libs
set(FBS_LIB_NAMES febioapp fscore gllib cuilib glwlib ogllib imagelib)

if (WIN32)
    set(FBS_DEBUG_LIBS "")
    set(FBS_RELEASE_LIBS "")

    foreach(name IN LISTS FBS_LIB_NAMES)
        find_library(TEMP NAMES ${name} 
            PATHS ${FBS_LIB_DIR} PATH_SUFFIXES Debug NO_DEFAULT_PATH)
        list(APPEND FBS_DEBUG_LIBS ${TEMP})
        unset(TEMP CACHE)
    endforeach(name)

    foreach(name IN LISTS FBS_LIB_NAMES)
        find_library(TEMP NAMES ${name} 
            PATHS ${FBS_LIB_DIR} PATH_SUFFIXES Release NO_DEFAULT_PATH)
        list(APPEND FBS_RELEASE_LIBS ${TEMP})
        unset(TEMP CACHE)
    endforeach(name)

    unset(FBS_LIB_NAMES)
else()
    set(FBS_LIBS "")

    foreach(name IN LISTS FBS_LIB_NAMES)
        findLib(FBS_LIB_DIR FBS_LIB ${name})
        list(APPEND FBS_LIBS ${FBS_LIB})
        unset(FBS_LIB)
    endforeach(name)

    unset(FBS_LIB_NAMES)
endif()

##### Stupid OMP stuff for stupid macOS #####

if(UNIX)
    option(USE_MKL_OMP "Link to MKL OMP instead of System OMP" OFF)
    set(MKL_OMP "" CACHE PATH "Path to the MKL OMP library")
    set(OMP_INC "" CACHE PATH "Path to the OMP include directory")
    mark_as_advanced(USE_MKL_OMP MKL_OMP OMP_INC)
endif()

##### Set appropriate defines and includes #####

if(WIN32)
    add_definitions(-DWIN32 -D_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/MP /openmp)
elseif(APPLE)
    add_definitions(-D__APPLE__)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
    
    if(${OpenMP_C_FOUND})
        add_compile_options(-Xpreprocessor -fopenmp -w)
    endif()
    add_compile_options(-w)
    
else()
	add_definitions(-DLINUX)
    add_compile_options(-w -Wall)
    
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
    
    if(${OpenMP_C_FOUND})
        add_compile_options(-fopenmp)
    endif()
    
    set(CMAKE_BUILD_RPATH_USE_LINK_PATH FALSE)
    set(CMAKE_BUILD_RPATH $ORIGIN/../lib/)
endif()

include_directories(.)

if(APPLE)
    include_directories(${GLEW_INCLUDE_DIR})
else()
    include_directories(${GLEW_INCLUDE_DIRS})
endif()

include_directories(${FEBio_INC})
include_directories(${FBS_INC})

##### Find Source Files #####

macro(findHdrSrc name)
	file(GLOB HDR_${name} "${name}/*.h")
	file(GLOB SRC_${name} "${name}/*.cpp")
endmacro()

findHdrSrc(FEBioApp)

##### Add binaries #####

if(WIN32)
	add_executable(${FBA_BIN_NAME} WIN32 ${HDR_FEBioApp} ${SRC_FEBioApp} febioapp.qrc)
	set_property(TARGET ${FBA_BIN_NAME} PROPERTY AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/AutoGen/FEBioApp_autogen)
    include(set_icon.cmake)
    set_icon(${FBA_BIN_NAME} icons/febioapp.ico)

elseif(APPLE)
	# set bundle icon
	set(FBS_ICON ${CMAKE_CURRENT_SOURCE_DIR}/icons/FEBioApp.icns)
	set(MACOSX_BUNDLE_ICON_FILE FEBioApp.icns)
	set_source_files_properties(${FBS_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	
	add_executable(${FBA_BIN_NAME} MACOSX_BUNDLE ${FBS_ICON} ${HDR_FEBioApp} ${SRC_FEBioApp} febioapp.qrc)
	set_property(TARGET ${FBA_BIN_NAME} PROPERTY AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/AutoGen/FEBioApp_autogen)

else()
	add_executable(${FBA_BIN_NAME} ${HDR_FEBioApp} ${SRC_FEBioApp} febioapp.qrc)
	set_property(TARGET ${FBA_BIN_NAME} PROPERTY AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/AutoGen/FEBioApp_autogen)
endif()


##### Link Libraries #####
# Linking the Qt libraries also pulls in the include files
target_link_libraries(${FBA_BIN_NAME} Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Network Qt6::OpenGL Qt6::OpenGLWidgets)

if(WIN32)
elseif(APPLE)
else()
    target_link_libraries(${FBA_BIN_NAME} -static-libstdc++ -static-libgcc)
    target_link_libraries(${FBA_BIN_NAME} -Wl,--start-group)
endif()

target_link_libraries(${FBA_BIN_NAME} ${OPENGL_LIBRARY})

target_link_libraries(${FBA_BIN_NAME} ${GLEW_LIBRARIES} GLEW::glew)

if (WIN32)
    foreach(name IN LISTS FEBio_RELEASE_LIBS)
        target_link_libraries(${FBA_BIN_NAME} optimized ${name})
    endforeach()

    foreach(name IN LISTS FEBio_DEBUG_LIBS)
        target_link_libraries(${FBA_BIN_NAME} debug ${name})
    endforeach()
else()
    target_link_libraries(${FBA_BIN_NAME} ${FEBio_LIBS})
endif()

if (WIN32)
    foreach(name IN LISTS FBS_RELEASE_LIBS)
        target_link_libraries(${FBA_BIN_NAME} optimized ${name})
    endforeach()

    foreach(name IN LISTS FBS_DEBUG_LIBS)
        target_link_libraries(${FBA_BIN_NAME} debug ${name})
    endforeach()
else()
    target_link_libraries(${FBA_BIN_NAME} ${FBS_LIBS})
endif()

if(WIN32)
    target_link_libraries(${FBA_BIN_NAME} vfw32.lib)
elseif(APPLE)
else()
    target_link_libraries(${FBA_BIN_NAME} -Wl,--end-group)
endif()

