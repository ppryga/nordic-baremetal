# This is a generic cmake compiler file. The file is aimed for bare matal ARM GCC toolchain

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# For bare-metal CMAKE doesn't have right linker script and dedicated flags, hecne complete compilation may fail.
# With this flag CMAKE uses add_library() w type STATIC, that skips linking step.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(TOOLCHAIN_PATH /usr/bin)

# Set toolchain executables
set(CMAKE_AR            ${TOOLCHAIN_PATH}/arm-none-eabi-ar${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_ASM_COMPILER  ${TOOLCHAIN_PATH}/arm-none-eabi-gcc${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_C_COMPILER    ${TOOLCHAIN_PATH}/arm-none-eabi-gcc${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_CXX_COMPILER  ${TOOLCHAIN_PATH}/arm-none-eabi-g++${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_LINKER        ${TOOLCHAIN_PATH}/arm-none-eabi-ld${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_OBJCOPY       ${TOOLCHAIN_PATH}/arm-none-eabi-objcopy${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_RANLIB        ${TOOLCHAIN_PATH}/arm-none-eabi-ranlib${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_SIZE          ${TOOLCHAIN_PATH}/arm-none-eabi-size${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_STRIP         ${TOOLCHAIN_PATH}/arm-none-eabi-strip${CMAKE_EXECUTABLE_SUFFIX})

#set(CMAKE_C_FLAGS       "-Wno-psabi --specs=nano.specs -fdata-sections -ffunction-sections -Wl,--gc-sections,--verbose" CACHE INTERNAL "")
set(CMAKE_C_FLAGS       "-Wno-psabi -fdata-sections -ffunction-sections -Wl,--gc-sections" CACHE INTERNAL "")
set(CMAKE_CXX_FLAGS     "${CMAKE_C_FLAGS} -fno-exceptions" CACHE INTERNAL "")

set(CMAKE_C_FLAGS_DEBUG     "-Os -g" CACHE INTERNAL "")
set(CMAKE_C_FLAGS_RELEASE   "-Os -DNDEBUG" CACHE INTERNAL "")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}" CACHE INTERNAL "")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}" CACHE INTERNAL "")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
