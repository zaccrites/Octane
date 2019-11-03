
# https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

TOOLCHAIN=arm-none-eabi
CPU=-mthumb -mcpu=cortex-m4

CCFLAGS= \
	$(CPU) -mthumb-interwork -mlong-calls \
	-std=gnu++14 -Os -pedantic \
	-fno-rtti -fno-exceptions \
	-ffunction-sections -fdata-sections \
	-nostdlib -nostartfiles -ffreestanding \
	-MMD -MP \
	-Wall -Wextra \
	-Wold-style-cast -Wnull-dereference -Wdouble-promotion \
	-Wlogical-op -Wuseless-cast -Wshadow \
	-Werror -Werror=switch \
	$(INCLUDES)

LINKER_SCRIPT=linker.ld
LDFLAGS= $(CPU) \
	--specs=nosys.specs \
	-T $(LINKER_SCRIPT) \
	-nostdlib -nostartfiles -ffreestanding

ASFLAGS=

BINFLAGS = \
	-j .isr_vector_table \
	-j .text \
	-j .data \
	-j .init_array \
	-O binary


CC=$(TOOLCHAIN)-g++
LD=$(TOOLCHAIN)-g++
AS=$(TOOLCHAIN)-as
OBJCOPY=$(TOOLCHAIN)-objcopy
OBJDUMP=$(TOOLCHAIN)-objdump
OPENOCD=openocd -f openocd/stm32f4discovery.cfg


SOURCE_DIR=src
BUILD_DIR=build


SOURCES = \
	$(SOURCE_DIR)/vectors.s \
	$(SOURCE_DIR)/main.cpp \
	$(SOURCE_DIR)/init.cpp \
	$(SOURCE_DIR)/stringio.cpp \
	$(SOURCE_DIR)/test.cpp

INCLUDES = \
	-isysteminclude/CMSIS/Core


OBJECTS = $(subst $(SOURCE_DIR),$(BUILD_DIR),$(SOURCES:=.o))


.PHONY: all
all : $(BUILD_DIR)/image.bin

DEPS := $(OBJECTS:.o=.d)
-include $(DEPS)

$(BUILD_DIR)/%.s.o : $(SOURCE_DIR)/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cpp.o : $(SOURCE_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -c $< -o $@

$(BUILD_DIR)/image.elf : $(OBJECTS) $(LINKER_SCRIPT)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

$(BUILD_DIR)/image.bin : $(BUILD_DIR)/image.elf
	$(OBJDUMP) -D $(BUILD_DIR)/image.elf > $(BUILD_DIR)/image.list
	$(OBJCOPY) $(BINFLAGS) $(BUILD_DIR)/image.elf $(BUILD_DIR)/image.bin

.PHONY: clean
clean:
	-@rm -rf $(BUILD_DIR)

.PHONY: install
install: $(BUILD_DIR)/image.bin
	$(OPENOCD) -c "program $(BUILD_DIR)/image.bin reset exit 0x08000000"
