cmake_minimum_required(VERSION 3.10)
project(capnzerowrapper)

# use c++-11 std
add_definitions(-std=c++11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# copying the variables makes it compatible for catkin to transitive forward the CapnProto libraries and include dirs
set(CapnProto_LIBRARIES ${CAPNP_LIBRARIES})
set(CapnProto_INCLUDE_DIRS ${CAPNP_INCLUDE_DIRS})

## System dependencies are found with CMake's conventions
find_package(ZeroMQ REQUIRED)
find_package(CapnProto REQUIRED)
find_package(Threads)

find_package(catkin QUIET)
if (catkin_FOUND)
    find_package(catkin REQUIRED COMPONENTS id_capnzero capnzero)
else(catkin_FOUND)
    set(catkin_LIBRARIES "id_capnzero capnzero")
endif (catkin_FOUND)

if (catkin_FOUND)
    catkin_package(
            INCLUDE_DIRS include
            CATKIN_DEPENDS id_capnzero capnzero
    )
endif(catkin_FOUND)

include_directories(
        include
        include/alica_capnz_msg
        ${ZeroMQ_INCLUDE_DIRS}
        ${CapnProto_INCLUDE_DIRS}
        ${capnzero_INCLUDE_DIRS}
        ${id_capnzero_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME}
        SHARED
            src/capnzerowrapper.cpp
            include/capnzerowrapper.h
        )

#### ALICA CAPNZERO MESSAGES
set(CAPNPC_IMPORT_DIRS "${id_capnzero_SOURCE_PREFIX}/include")# include external capnp message schema files
set(${PROJECT_NAME}_msgs "${CMAKE_CURRENT_SOURCE_DIR}/include/alica_capnz_msg")
file(GLOB_RECURSE capnp_messages
        #${CMAKE_CURRENT_SOURCE_DIR}/include/capnzero/*.capnp
        ${${PROJECT_NAME}_msgs}/ID.capnp
        ${${PROJECT_NAME}_msgs}/*.capnp)
set(CAPNPC_SRC_PREFIX ${${PROJECT_NAME}_msgs})
set(CAPNPC_OUTPUT_DIR ${${PROJECT_NAME}_msgs})
capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS ${capnp_messages})

add_library(alica_capnz_msgs ${CAPNP_SRCS})

target_link_libraries(${PROJECT_NAME}   
        PRIVATE
                capnzero
                capnzero-base-msgs
                alica_capnz_msgs
                ${ZeroMQ_LIBRARIES}
                ${CAPNP_LIBRARIES}
                ${capnzero_LIBRARIES}
                ${id_capnzero_LIBRARIES}
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(${PROJECT_NAME} PUBLIC ZMQ_BUILD_DRAFT_API=1)

add_executable(capnzerowrapper_sub src/capnzerowrapper.cpp)
target_link_libraries(capnzerowrapper_sub
        PRIVATE
        capnzero
        capnzero-base-msgs
        alica_capnz_msgs
        ${ZeroMQ_LIBRARIES}
        ${CAPNP_LIBRARIES}
        ${capnzero_LIBRARIES}
        ${id_capnzero_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)