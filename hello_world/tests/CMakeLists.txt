#
# Copyright (c) 2023 Piotr Pryga
#
# SPDX-License-Identifier: Apache-2.0
#
cmake_minimum_required(VERSION 3.15.3)

project(hello-world-bare-metal) 

if (DEFINED ENV{CPPUTEST_HOME})
        message(STATUS "Using CppUTest from: $ENV{CPPUTEST_HOME}")
        set(CPPUTEST_INCLUDE_DIRS $ENV{CPPUTEST_HOME}/include)
        set(CPPUTEST_LIBRARIES $ENV{CPPUTEST_HOME}/lib)
        set(CPPUTEST_LDFLAGS CppUTest CppUTestExt)
else()
        find_package(PkgConfig REQUIRED)
        pkg_search_module(CPPUTEST REQUIRED cpputest)
        message(STATUS "Found CppUTest vesion ${CPPUTEST_VERSION}")
endif()

set(TEST_EXECUTABLE ${EXECUTABLE}_tests)
set(TEST_SRC_FILES  
        ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/slist_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../tools/slist.c)

add_executable(${TEST_EXECUTABLE} ${TEST_SRC_FILES})

#target_compile_definitions(${TEST_EXECUTABLE} )

target_include_directories(${TEST_EXECUTABLE} PRIVATE
        ${CPPUTEST_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/../
        ${CMAKE_CURRENT_SOURCE_DIR}/../tools)

target_link_directories(${TEST_EXECUTABLE} PRIVATE ${CPPUTEST_LIBRARIES})
target_link_libraries(${TEST_EXECUTABLE} PRIVATE ${CPPUTEST_LDFLAGS})