cmake_minimum_required(VERSION 3.0)
project(TressFx)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Utils.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake)


# OPTIONS
set_with_default(CMAKE_BUILD_TYPE Release STRING "Choose the type of build (Debug or Release)")
assert_one_of(CMAKE_BUILD_TYPE Release Debug)


# VERSION
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)


# Print current project settings
message("CMAKE_GENERATOR: ${CMAKE_GENERATOR}")
message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message("Architecture: ${M_ARCH_NAME}")
message("Complier: ${M_COMPILER_NAME}")
message("Project: ${CMAKE_PROJECT_NAME} v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")


# EXECUTABLE
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# do not include "src/main.cpp" in SOURCE_FILES, as it conflicts with tests
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
file(GLOB_RECURSE LIBS_FILES "libs/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${LIBS_FILES} "src/main.cpp")
# add_executable(${PROJECT_NAME} ${SOURCE_FILES} "src/main.cpp")

# set(TESTS_TARGET "tests")
# file(GLOB_RECURSE TEST_FILES "tests/*.cpp")
# add_executable(${TESTS_TARGET} EXCLUDE_FROM_ALL ${SOURCE_FILES} ${TEST_FILES})


# COMPILER & LINKER
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic ${M_CPP_17_COMPILER_FLAG}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem \"libs/amd_tressfx/include\"") # add TFx to include dirs (required by AMD lib impl)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -D DEBUG -O0 -gdwarf-2") # flag for debug
endif()

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=c++1z -lstdc++" )
target_link_libraries(${PROJECT_NAME} ${M_ADDITIONAL_LIBS})
# target_link_libraries(${TESTS_TARGET} ${M_ADDITIONAL_LIBS})


# LIBS
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

find_package(GLM REQUIRED)
include_directories(PUBLIC ${GLM_INCLUDE_DIRS})

find_package(SDL2 REQUIRED)
include_directories(PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
# target_link_libraries(${TESTS_TARGET} ${SDL2_LIBRARY})

find_package(GLAD REQUIRED)
include_directories(PUBLIC ${GLAD_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${GLAD_LIBRARY})
# target_link_libraries(${TESTS_TARGET} ${GLAD_LIBRARY})

# PREPROCESSOR
#add_definitions(-DUSE_BROFILER)


# Define Options
# option(OPTION_ENABLE_BROFILER "Enable Brofiler profiler support" ON)


#set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")

#if (OPTION_ENABLE_BROFILER)
#	add_definitions(-DUSE_BROFILER)
#	include_directories( ${SDK_DIR}/Brofiler )
#	link_directories( ${SDK_DIR}/Brofiler )
#	set(COMMON_LIBS ${COMMON_LIBS} ProfilerCore64)
#endif()

#if (WIN32)
#	add_subdirectory(Code/Launcher/DedicatedLauncher)
#	add_subdirectory(Code/Launcher/WindowsLauncher)
#endif ()
