
# https://superuser.com/questions/1400541/cmake-on-arm-arm-none-eabi-configuration



set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)


set(TARGET_ARCH_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mabi=aapcs
    # -mfpu=fpv4-sp-d16       # TODO (these options don't seem to be right)
    # -mfloat-abi=hard
    -mlong-calls
    -mno-unaligned-access
)

set(TARGET_COMPILE_FLAGS
    "${TARGET_ARCH_FLAGS}"
    -fno-rtti
    -fno-exceptions
    -fno-non-call-exceptions
    -fno-use-cxa-atexit
    -ffunction-sections
    -fdata-sections
    -ffreestanding
)


# https://stackoverflow.com/questions/53633705/cmake-the-c-compiler-is-not-able-to-compile-a-simple-test-program
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")


# https://stackoverflow.com/questions/11594905/how-do-i-correctly-pass-cmake-list-semicolon-sep-of-flags-to-set-target-proper
set(LINKER_FLAGS
    ${TARGET_ARCH_FLAGS}
    -nostartfiles
    --specs=nano.specs
    --specs=nosys.specs
    -T "${CMAKE_SOURCE_DIR}/firmware/linker.ld"
)
string(REPLACE ";" " " CMAKE_EXE_LINKER_FLAGS "${LINKER_FLAGS}")


