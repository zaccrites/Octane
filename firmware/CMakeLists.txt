
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# TODO: Rearrange CMakeLists
# https://stackoverflow.com/questions/9542971/using-cmake-with-multiple-compilers-for-the-same-language

# Will need to run CMake multiple times, since cross compiling isn't an option.
# One can do simulator, one can do firmware. Possibly a third can do bitstream generation?
# Can wrap them in a script to build whatever without worrying which is which.
# Can also run cmake itself the first time.

# TODO: Inject these as -D options on the command line when calling
# cmake the first time? Even more of a reason to script it.

# cmake -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake ...

# Idea: can I pass the option on the command line and use the same CMakeLists file?
# I can define variables to decide which BSP directory to use, which toolchain
# to use, etc. The FPGA stuff can live in "both" (ie, doesn't care about those variables),
# since it doesn't care which software toolchain is used.

project(octane_firmware VERSION 0.1.0 LANGUAGES CXX C ASM)


add_executable(firmware
    "src/main.cpp"
    "src/init.cpp"
    "src/Fpga.cpp"
    "src/AudioOut.cpp"

    "src/CriticalSection.cpp"

    "src/printf.cpp"
    "src/isr.cpp"
    "src/start.cpp"
    "src/vectors.s"

)

target_include_directories(firmware SYSTEM PRIVATE
    "include/CMSIS"
    "include/device"
)

target_link_libraries(firmware
    printf
)


# TODO: Are all the binutils available like normal once the toolchain is setup?
# add_custom_target(firmware_image DEPENDS firmware.bin)
add_custom_command(
    TARGET firmware POST_BUILD
    COMMENT "Creating firmware image"
    COMMAND ${CMAKE_OBJDUMP} -D firmware > firmware.list
    COMMAND
        ${CMAKE_OBJCOPY} firmware firmware.bin
        -j .isr_vector_table
        -j .text
        -j .data
        -j .init_array
        -O binary
)


# TODO: Improve this, possibly as a separate script to flash firmware with OpenOCD.
# Will eventually need to support a mode to flash directly over USB or WiFi,
# as well as to download e.g. FPGA bitstream directly to write to a separate flash
# (if not enough flash inside the microcontroller itself)
add_custom_target(flash
    COMMAND openocd
        -f "${CMAKE_SOURCE_DIR}/openocd/stm32f4discovery.cfg"
        -c "program ${CMAKE_BINARY_DIR}/firmware.bin reset exit 0x08000000"
    DEPENDS firmware
    COMMENT "Flashing development board"
)


target_compile_definitions(firmware PRIVATE
    STM32F407xx
)


target_compile_options(firmware PRIVATE
    -mcpu=cortex-m4
    -mthumb
    -mabi=aapcs
    # -mfpu=fpv4-sp-d16       # TODO (these options don't seem to be right)
    # -mfloat-abi=hard
    -mlong-calls
    -mno-unaligned-access

    -std=gnu++17
    -pedantic

    -Og
    -g3
    # -O2

    -fno-rtti
    -fno-exceptions
    -fno-non-call-exceptions
    -fno-use-cxa-atexit
    -fdiagnostics-color
    -ffunction-sections
    -fdata-sections
    -ffreestanding

    -Wall
    -Wextra
    -Weffc++
    -Wold-style-cast
    -Wnull-dereference
    -Wdouble-promotion
    -Wlogical-op
    -Wuseless-cast
    -Wshadow
    -Werror=switch
)





add_library(printf
    "../vendor/printf/printf.c"
)

# TODO: Address this. Do I need some common options?
target_compile_options(printf PRIVATE
    -mcpu=cortex-m4
    -mthumb
    -mabi=aapcs
    # -mfpu=fpv4-sp-d16       # TODO (these options don't seem to be right)
    # -mfloat-abi=hard
    -mlong-calls

    -Og
    -g3
    # -O2
)

target_include_directories(printf SYSTEM PUBLIC
    "../vendor/printf"
)
