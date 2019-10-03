cmake_minimum_required(VERSION 3.9)
project(capnzerowrapper VERSION 1.0 DESCRIPTION "Wrapper library for Capnzero")

## Compile as C++11
add_compile_options(-std=c++14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(submodules/capnzero/capnzero)

include(GNUInstallDirs)

# Define where to find modules for ZeroMQ
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake CACHE INTERNAL "" FORCE)

# copying the variables makes it compatible for catkin to transitive forward the CapnProto libraries and include dirs
set(CapnProto_LIBRARIES ${CAPNP_LIBRARIES})
set(CapnProto_INCLUDE_DIRS ${CAPNP_INCLUDE_DIRS})

## System dependencies are found with CMake's conventions
find_package(ZeroMQ REQUIRED)
find_package(CapnProto REQUIRED)

## Declare a C++ library
add_library(${PROJECT_NAME}
        SHARED
            src/capnzerowrapper.cpp
            include/capnzerowrapper.h
        )

# To fix compilation problem: relocation R_X86_64_32 against `a local symbol' can not be
# used when making a shared object; recompile with -fPIC
# See http://www.cmake.org/pipermail/cmake/2007-May/014350.html
#
IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fPIC")
ENDIF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )


target_link_libraries(${PROJECT_NAME}
        PRIVATE
            capnzero
            capnzero-base-msgs
            ${ZeroMQ_LIBRARIES}
            ${CAPNP_LIBRARIES}
        )

target_include_directories(${PROJECT_NAME}
        PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}/include"
            submodules/capnzero/capnzero/include
            submodules/capnzero/capnzero/include/capnzero-base-msgs)

include_directories(
        include
        capnzero
        ${ZeroMQ_INCLUDE_DIRS}
        ${CapnProto_INCLUDE_DIRS}
)

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

target_compile_definitions(${PROJECT_NAME} PUBLIC ZMQ_BUILD_DRAFT_API=1)
