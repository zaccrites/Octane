
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



# TODO: Figure out which auto generated files are useful and copy them
# into the proper place. Delete the others. I can always run the tool again
# to regenerate them if I need them. Mostly I just want the headers
# and basic init code to use as a template from which I will re-write.
# Or maybe use them to get a basic blinky program working, then rewrite them.
add_executable(firmware
    # "startup_stm32f407xx.s"
    # "src/system_stm32f4xx.c"
    "src/vectors.s"
    "src/init.c"
    "src/main.c"
)

target_include_directories(firmware PRIVATE SYSTEM
    "include/CMSIS"
    "include/device"
)


# # TODO: Are all the binutils available like normal once the toolchain is setup?
# # add_custom_target(firmware_image DEPENDS firmware.bin)
# add_custom_command(
#     TARGET firmware POST_BUILD
#     COMMENT "Creating firmware image"
#     COMMAND ${CMAKE_OBJDUMP} -D firmware > firmware.list
#     COMMAND
#         ${CMAKE_OBJCOPY} firmware firmware.bin
#         -j .isr_vector -j .text -j .data
#         -O binary
# )

# # TODO: Are all the binutils available like normal once the toolchain is setup?
# # add_custom_target(firmware_image DEPENDS firmware.bin)
# add_custom_command(
#     TARGET firmware POST_BUILD
#     COMMENT "Creating firmware image"
#     COMMAND ${CMAKE_OBJDUMP} -D firmware > firmware.list
#     COMMAND
#         ${CMAKE_OBJCOPY} firmware firmware.bin
#         -j .isr_vector_table
#         -j .text
#         -j .rodata
#         -j .preinit_array
#         -j .init_array
#         -j .fini_array
#         -j .ARM
#         -j .ARM.extab
#         -O binary
# )



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










# TODO: Separate script to flash firmware with OpenOCD.
# Will eventually need to support a mode to flash directly over USB or WiFi,
# as well as to download e.g. FPGA bitstream directly to write to a separate flash
# (if not enough flash inside the microcontroller itself)


target_compile_definitions(firmware PRIVATE
    STM32F407xx
)



# target_compile_options(firmware PRIVATE
#     -Wall -Wextra -O0 -std=gnu11 -pedantic
#     -mthumb -mcpu=cortex-m4
#     -nostdlib -nostartfiles -ffreestanding
#     -ffunction-sections -fdata-sections
# )

target_compile_options(firmware PRIVATE
    -mthumb -mcpu=cortex-m4 -mthumb-interwork -mlong-calls
    -std=gnu11 -O2 -pedantic
    -fno-rtti -fno-exceptions
    -ffunction-sections -fdata-sections
    -nostdlib -nostartfiles -ffreestanding
    -MMD -MP
    -Wall -Wextra
    -Wold-style-cast -Wnull-dereference -Wdouble-promotion
    -Wlogical-op -Wuseless-cast -Wshadow
    -Werror=switch
)







# # TODO: Prevent copy-paste of compiler settings
# target_compile_options(firmware PRIVATE

#     -mcpu=cortex-m4
#     -mthumb
#     -mthumb-interwork
#     -mlong-calls

#     -ffunction-sections
#     -fdata-sections
#     -ffreestanding

#     # TODO: Remove these
#     -nostdlib
#     -nostartfiles

#     # -fno-rtti
#     # -fno-exceptions



#     -pedantic
#     -Wall
#     -Wextra
#     # -Werror
#     # -Weffc++
#     -Wshadow
#     -Wcast-qual
#     -Wold-style-cast
#     # -Wfloat-equal
#     -Wnull-dereference
#     -Wdouble-promotion
#     -Wlogical-op
#     -Wuseless-cast

#     -Werror=switch
#     -Werror=return-type
#     -Werror=uninitialized
#     # -Werror=sometimes-uninitialized

#     -fdiagnostics-color

#     -O2

# )
