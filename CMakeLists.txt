cmake_minimum_required(VERSION 3.5)
project(DataStructures)

SET(DEFAULT_BUILD_TYPE "Debug")
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
ENDIF()
SET(EXPECTED_BUILD_TYPES Debug Release RelWithDebInfo MinSizeRel)
IF("${CMAKE_BUILD_TYPE}" IN_LIST EXPECTED_BUILD_TYPES)
    MESSAGE(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
ELSE()
    MESSAGE(WARNING "Unexcpected build type: ${CMAKE_BUILD_TYPE}")
ENDIF()

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_C_STANDARD 11)
SET(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Werror")
SET(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Werror")

INCLUDE_DIRECTORIES(. ./common ./engine ./structs)

file(GLOB SOURCES
        "${PROJECT_SOURCE_DIR}/common/*.h"
        "${PROJECT_SOURCE_DIR}/common/*.hpp"
        "${PROJECT_SOURCE_DIR}/common/*.cpp"
        "${PROJECT_SOURCE_DIR}/common/*.c"
        "${PROJECT_SOURCE_DIR}/engine/*.h"
        "${PROJECT_SOURCE_DIR}/engine/*.hpp"
        "${PROJECT_SOURCE_DIR}/engine/*.cpp"
        "${PROJECT_SOURCE_DIR}/engine/*.c"
        "${PROJECT_SOURCE_DIR}/structs/*.h"
        "${PROJECT_SOURCE_DIR}/structs/*.hpp"
        "${PROJECT_SOURCE_DIR}/structs/*.cpp"
        "${PROJECT_SOURCE_DIR}/structs/*.c"
        )

ADD_EXECUTABLE(DataStructures DataStructures.cpp ${SOURCES})
TARGET_LINK_LIBRARIES(DataStructures dl)
