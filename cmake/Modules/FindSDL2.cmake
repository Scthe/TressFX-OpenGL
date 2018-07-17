# Defines on success:
#   SDL2_INCLUDE_DIRS - where to find SDL2/SDL.h
#   SDL2_FOUND        - if the library was successfully located
#   SDL2_LIBRARY      - the name of the library to link against
#
# Parameters:
#   SDL2_ROOT_DIR     - root directory of a SDL2 installation

# Based on:
# - https://github.com/brendan-w/collector/blob/master/cmake/FindSDL2.cmake
# - https://github.com/tcbrindle/sdl2-cmake-scripts/blob/master/FindSDL2.cmake

IF(M_OS_LINUX)
    SET(_sdl2_SEARCH_DIRS "/usr" "/usr/local")
ENDIF()

# Put user specified location at beginning of search
IF(SDL2_ROOT_DIR)
    SET(_sdl2_SEARCH_DIRS "${SDL2_ROOT_DIR}" ${_sdl2_SEARCH_DIRS})
ENDIF()

# Locate header
FIND_PATH(SDL2_INCLUDE_DIRS "SDL2/SDL.h"
    PATH_SUFFIXES "include"
    HINTS $ENV{SDL2_ROOT_DIR}
    PATHS ${_sdl2_SEARCH_DIRS})

# Locate libraries: SDL2 & SDL2main
FIND_LIBRARY(SDL2_LIBRARY_DIR
	NAMES SDL2
	PATH_SUFFIXES lib64 lib
	HINTS $ENV{SDL2_ROOT_DIR}
	PATHS ${_sdl2_SEARCH_DIRS}
)
FIND_LIBRARY(SDL2MAIN_LIBRARY
	NAMES SDL2main
	PATH_SUFFIXES lib64 lib
	HINTS $ENV{SDL2_ROOT_DIR}
	PATHS ${_sdl2_SEARCH_DIRS}
)

SET(SDL2_LIBRARY ${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY_DIR} ${SDL2_LIBRARY})

# mingw requires additional prepended library: mingw32
IF(MINGW)
	SET(SDL2_LIBRARY mingw32 ${SDL2_LIBRARY})
ENDIF()


# Handle REQUIRED argument, define *_FOUND variable
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2 DEFAULT_MSG
    SDL2_INCLUDE_DIRS SDL2MAIN_LIBRARY SDL2_LIBRARY_DIR)

IF(SDL2_FOUND)
    MESSAGE(STATUS "SDL2_INCLUDE_DIRS = ${SDL2_INCLUDE_DIRS}")
    MESSAGE(STATUS "SDL2_LIBRARY = ${SDL2_LIBRARY}")
ENDIF()
