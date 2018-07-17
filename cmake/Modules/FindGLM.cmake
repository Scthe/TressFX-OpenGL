# Defines on success:
#   GLM_INCLUDE_DIRS  - where to find glm/glm.hpp
#   GLM_FOUND         - if the library was successfully located
#
# Parameters:
#   GLM_ROOT_DIR      - root directory of a glm installation
#                       Headers are expected to be found in either:
#                       <GLM_ROOT_DIR>/glm/glm.hpp           OR
#                       <GLM_ROOT_DIR>/include/glm/glm.hpp
#                       This variable can either be a cmake or environment
#                       variable.

# Based on https://github.com/Groovounet/glm-deprecated/blob/master/util/FindGLM.cmake

IF(M_OS_LINUX)
    SET(_glm_HEADER_SEARCH_DIRS "/usr/include" "/usr/local/include")
ENDIF()

# Put user specified location at beginning of search
IF(GLM_ROOT_DIR)
    SET(_glm_HEADER_SEARCH_DIRS "${GLM_ROOT_DIR}" ${_glm_HEADER_SEARCH_DIRS})
ENDIF()

# Locate header
FIND_PATH(GLM_INCLUDE_DIRS "glm/glm.hpp"
    PATH_SUFFIXES "include"
    HINTS $ENV{GLAD_ROOT_DIR}
    PATHS ${_glm_HEADER_SEARCH_DIRS})

# Handle REQUIRD argument, define *_FOUND variable
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLM DEFAULT_MSG
    GLM_INCLUDE_DIRS)

IF(GLM_FOUND)
    MESSAGE(STATUS "GLM_INCLUDE_DIRS = ${GLM_INCLUDE_DIRS}")
ENDIF()