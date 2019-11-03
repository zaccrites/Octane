
# https://superuser.com/questions/1400541/cmake-on-arm-arm-none-eabi-configuration

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

# set(CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs -T \"${CMAKE_SOURCE_DIR}/STM32F407VGTx_FLASH.ld\" -nostdlib -nostartfiles -ffreestanding")
# set(CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs -T \"${CMAKE_SOURCE_DIR}/STM32F407VGTx_FLASH.ld\"")




# set(CMAKE_EXE_LINKER_FLAGS "-mcpu=cortex-m4 -mthumb --specs=nano.specs -T \"${CMAKE_SOURCE_DIR}/STM32F407VGTx_FLASH.ld\" -lc -lnosys -nostdlib -nostartfiles -ffreestanding")
set(CMAKE_EXE_LINKER_FLAGS "-mcpu=cortex-m4 -mthumb --specs=nosys.specs -T \"${CMAKE_SOURCE_DIR}/linker.ld\" -nostdlib -nostartfiles -ffreestanding")


