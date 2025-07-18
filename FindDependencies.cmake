# Qt
find_package(Qt6 COMPONENTS Widgets Gui Network OpenGL OpenGLWidgets REQUIRED) 

mark_as_advanced(Qt6_DIR Qt6Core_DIR Qt6CoreTools_DIR Qt6DBusTools_DIR Qt6DBus_DIR Qt6GuiTools_DIR 
    Qt6Gui_DIR Qt6Network_DIR Qt6OpenGLWidgets_DIR Qt6OpenGL_DIR Qt6WidgetsTools_DIR Qt6Widgets_DIR)

# FEBio
# Find FEBio SDK or git repo automatically
if(WIN32)
	set(TEMP_PATHS ${CMAKE_SOURCE_DIR}/.. ${CMAKE_SOURCE_DIR}/../.. $ENV{HOMEPATH}/ $ENV{HOMEPATH}/source/repos $ENV{HOMEPATH}/*)
else()
    set(TEMP_PATHS ${CMAKE_SOURCE_DIR}/.. ${CMAKE_SOURCE_DIR}/../.. $ENV{HOME}/ $ENV{HOME}/*)
endif()

find_path(FEBio_SDK FECore/stdafx.h
    PATHS ${TEMP_PATHS}
    PATH_SUFFIXES FEBio
    DOC "Path to the FEBio SDK, or git repo.")

if(NOT FEBio_SDK)
    if(WIN32)
        set(TEMP_PATHS $ENV{PROGRAMFILES}/* ${CMAKE_SOURCE_DIR}/.. $ENV{HOMEPATH}/* )
    elseif(APPLE)
        set(TEMP_PATHS /Applications/* ${CMAKE_SOURCE_DIR}/.. $ENV{HOME}/*)
    else()
        set(TEMP_PATHS ${CMAKE_SOURCE_DIR}/.. $ENV{HOME}/*)
    endif()

    find_path(FEBio_SDK "include/FECore/stdafx.h"
        PATHS ${TEMP_PATHS}
        PATH_SUFFIXES sdk
        DOC "Path to the FEBio SDK, or git repo.")
endif()

if(NOT FEBio_SDK)
    set(FEBio_SDK "" CACHE PATH "Path to the FEBio SDK, or git repo.")
    message(FATAL_ERROR "Unable to find path to FEBio SDK or git repo automatically. Please set FEBio_SDK to the path to your FEBio SDK or git repo.")
endif()

# Only update the include and lib directories if the FEBio_SDK path has been changed.
if(NOT OLD_SDK)
    set(NEWPATH TRUE)
else()
    #cmake_path(CONVERT ${OLD_SDK} TO_CMAKE_PATH_LIST STD_OLD_SDK)
    #cmake_path(CONVERT ${FEBio_SDK} TO_CMAKE_PATH_LIST STD_FEBIO_SDK)
    #string(COMPARE NOTEQUAL ${STD_FEBIO_SDK} ${STD_OLD_SDK} NEWPATH)
    string(COMPARE NOTEQUAL ${FEBio_SDK} ${OLD_SDK} NEWPATH)
endif()

if(NEWPATH)
    # Is this the SDK?
    string(REGEX MATCH "sdk" IS_SDK ${FEBio_SDK})

    set(LIB_SUFFIXES "")
    if(IS_SDK)
        set(FEBio_INC "${FEBio_SDK}/include" CACHE PATH "Path to FEBio include directory." FORCE)

        if(WIN32)
            list(APPEND LIB_SUFFIXES "lib" "lib/Release"  "vs2017/Release" "vs2017/Debug")
        else()
            list(APPEND LIB_SUFFIXES "lib")
        endif()
    else()
        set(FEBio_INC ${FEBio_SDK} CACHE PATH "Path to FEBio include directory." FORCE)

        if(WIN32)
            list(APPEND LIB_SUFFIXES "lib" "lib/Release" "cmbuild/lib/Release" "cmbuild/lib/Debug" "cbuild/lib/Release" "cbuild/lib/Debug" "build/lib/Release" "build/lib/Debug")
        else()
            list(APPEND LIB_SUFFIXES "cbuild/lib" "cmbuild/lib" "build/lib" "cbuild/Release/lib" "cmbuild/Release/lib" "build/Release/lib" "cbuild/Debug/lib" "cmbuild/Debug/lib" "build/Debug/lib")
        endif()
    endif()

    mark_as_advanced(FEBio_INC)

    # Find lib path
    find_library(FECORE
        NAMES FECore fecore fecore_gcc64 fecore_lnx64
        PATHS ${FEBio_SDK}
        PATH_SUFFIXES ${LIB_SUFFIXES}
        DOC "FEBio library path")

    if(FECORE)
        get_filename_component(FECORE_TEMP ${FECORE} DIRECTORY)
        set(FEBio_LIB_DIR ${FECORE_TEMP} CACHE PATH "Path to the FEBio lib directory." FORCE)
        mark_as_advanced(FEBio_LIB_DIR)
        unset(FECORE_TEMP)
        unset(FECORE CACHE)
    else()
        set(FEBio_LIB_DIR CACHE PATH "Path to the FEBio lib directory." FORCE)
        message(SEND_ERROR "Unable to find FEBio Library path automatically. Set FEBio_LIB_DIR.")
        unset(FECORE CACHE)
    endif()
endif()

set(OLD_SDK ${FEBio_SDK} CACHE INTERNAL "Old SDK path.")
mark_as_advanced(OLD_SDK)

set(FBS_INC CACHE PATH "Path to FEBio Studio source")
set(FBS_LIB_DIR CACHE PATH "Path to the FEBio lib directory.")

# OpenGL
find_package(OpenGL REQUIRED)
# Use non-standard PATH_SUFFIXES on HOMEBREW macs
option(HOMEBREW "Did you use HOMEBREW to install dependencies" OFF)
if(HOMEBREW AND APPLE)
  find_package(GLEW PATH_SUFFIXES glew REQUIRED)
else()
  find_package(GLEW REQUIRED)
endif()

# OpenMP
if(NOT WIN32)
    find_package(OpenMP QUIET)
endif()
