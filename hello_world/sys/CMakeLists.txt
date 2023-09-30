cmake_minimum_required(VERSION 3.15.3)

# Optional: print out extra messages to see what is going on. Comment it to have less verbose messages
set(CMAKE_VERBOSE_MAKEFILE ON)

set(LIB_NAME sys)

# List of source files
set(SRC_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/mutex.c
        ${CMAKE_CURRENT_SOURCE_DIR}/pend_sv.S
        ${CMAKE_CURRENT_SOURCE_DIR}/scheduler.c
        ${CMAKE_CURRENT_SOURCE_DIR}/spin_lock.c
        ${CMAKE_CURRENT_SOURCE_DIR}/syscalls.c
        ${CMAKE_CURRENT_SOURCE_DIR}/thread.c 
        )

# Set a library as interface. It is not compiled separately but allows to set properies for the target.
add_library(${LIB_NAME} INTERFACE "")

target_sources(${LIB_NAME} INTERFACE ${SRC_FILES})

# Append the library to global LIBS_ALL property to be added to link libraries for final target
set_property(GLOBAL APPEND PROPERTY LIBS_ALL ${LIB_NAME})
