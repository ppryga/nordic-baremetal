cmake_minimum_required(VERSION 3.15.3)

# Optional: print out extra messages to see what is going on. Comment it to have less verbose messages
set(CMAKE_VERBOSE_MAKEFILE ON)

# Path to toolchain file. This one has to be before 'project()' below
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/arm-none-eabi.cmake)
set(CMSIS_PATH ${CMAKE_SOURCE_DIR}/../lib/cmsis/CMSIS/)
set(CMSIS_CORE_INCLUDE_PATH ${CMSIS_PATH}/Core/Include)

set(NRFX_PATH ${CMAKE_SOURCE_DIR}/../lib/nrfx)
set(MDK_PATH ${NRFX_PATH}/mdk)
set(LD_SCRIPT_PATH ${MDK_PATH})

set(NRFX_DRIVERS_PATH ${NRFX_PATH}/drivers)
set(NRFX_DRIVERS_SRC_PATH ${NRFX_DRIVERS_PATH}/src)
set(NRFX_DRIVERS_INCLUDE_PATH ${NRFX_DRIVERS_PATH}/include)

set(NRF_DRIVERS_FILES ${NRFX_DRIVERS_SRC_PATH}/nrfx_systick.c
        ${NRFX_DRIVERS_SRC_PATH}/nrfx_uarte.c)

# Setup project, output and linker file
project(hello-world-bare-metal) 
set(EXECUTABLE ${PROJECT_NAME}.elf)
set(LINKER_FILE ${LD_SCRIPT_PATH}/nrf52833_xxaa.ld)

enable_language(C ASM)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Optional: issue a message to be sure it uses the correct toolchain file.
message(STATUS "CMAKE_TOOLCHAIN_FILE is: ${CMAKE_TOOLCHAIN_FILE}")

# List of source files
set(SRC_FILES
        ${MDK_PATH}/gcc_startup_nrf52833.S
        ${MDK_PATH}/system_nrf52833.c
        ${CMAKE_SOURCE_DIR}/main.c 
        ${NRF_DRIVERS_FILES}
        )

# Build the executable based on the source files
add_executable(${EXECUTABLE} ${SRC_FILES})

# List of compiler defines, prefix with -D compiler option
target_compile_definitions(${EXECUTABLE} PRIVATE
        -DNRF52
        -DNRF52_SERIES
        -DNRF52833_XXAA
        -D__STARTUP_CLEAR_BSS # Don't know why C startup doesn't clear BSS on startup
        )

# List of includ directories
target_include_directories(${EXECUTABLE} PRIVATE
        ${NRFX_PATH}
        ${MDK_PATH}
        ${NRFX_DRIVERS_INCLUDE_PATH}
        ${CMSIS_CORE_INCLUDE_PATH}
        ${CMAKE_SOURCE_DIR}
        )

# Compiler options
target_compile_options(${EXECUTABLE} PRIVATE
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard

        -fdata-sections
        -ffunction-sections

        -Wall
        -O0
        -g3
        )

# Linker options
target_link_options(${EXECUTABLE} PRIVATE
        -T${LINKER_FILE}
        -L${LD_SCRIPT_PATH}
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        -nodefaultlibs
        #-specs=nano.specs 
        -lc
        -lgcc
        #-lnosys
        -Wl,-Map=${PROJECT_NAME}.map,--cref
        -Wl,--gc-sections
        -Xlinker -print-memory-usage -Xlinker
        #--verbose
        )

# Optional: Print executable size as part of the post build process
add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND ${CMAKE_SIZE} ${EXECUTABLE})

# Optional: Create hex, bin and S-Record files after the build
add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O srec --srec-len=64 ${EXECUTABLE} ${PROJECT_NAME}.s19
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE} ${PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE} ${PROJECT_NAME}.bin)
