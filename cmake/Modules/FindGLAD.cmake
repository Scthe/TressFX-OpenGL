# Defines on success:
#   GLAD_INCLUDE_DIRS - where to find SDL2/SDL.h
#   GLAD_FOUND        - if the library was successfully located
#   GLAD_LIBRARY      - the name of the library to link against
#
# Parameters:
#   GLAD_ROOT_DIR     - root directory of a GLAD installation

IF(M_OS_LINUX)
    SET(_glad_SEARCH_DIRS "/usr" "/usr/local")
ENDIF()

# Put user specified location at beginning of search
IF(GLAD_ROOT_DIR)
    SET(_glad_SEARCH_DIRS "${GLAD_ROOT_DIR}" ${_glad_SEARCH_DIRS})
ENDIF()

# Locate header
FIND_PATH(GLAD_INCLUDE_DIRS "glad/glad.h"
    PATH_SUFFIXES "include"
    HINTS $ENV{GLAD_ROOT_DIR}
    PATHS ${_glad_SEARCH_DIRS})

# Locate libraries
FIND_LIBRARY(GLAD_LIBRARY
	NAMES GLAD
	PATH_SUFFIXES lib64 lib
	HINTS $ENV{GLAD_ROOT_DIR}
	PATHS ${_glad_SEARCH_DIRS}
)

# Handle REQUIRED argument, define *_FOUND variable
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLAD DEFAULT_MSG
    GLAD_INCLUDE_DIRS GLAD_LIBRARY)

IF(GLAD_FOUND)
    MESSAGE(STATUS "GLAD_INCLUDE_DIRS = ${GLAD_INCLUDE_DIRS}")
    MESSAGE(STATUS "GLAD_LIBRARY = ${GLAD_LIBRARY}")
ENDIF()
