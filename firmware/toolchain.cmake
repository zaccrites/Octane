
# https://superuser.com/questions/1400541/cmake-on-arm-arm-none-eabi-configuration

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)


# https://stackoverflow.com/questions/53633705/cmake-the-c-compiler-is-not-able-to-compile-a-simple-test-program
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

set(CMAKE_EXE_LINKER_FLAGS "-nostartfiles -specs=nano.specs -specs=nosys.specs -T \"${CMAKE_SOURCE_DIR}/linker.ld\"")



