# filenames
EXECUTABLE=firmware.elf
BIN_IMAGE=firmware.bin

# toolchain
CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

# debugging and optimization flags
CFLAGS   = -g -O1 -DSTM32 -DSTM32F4 -DSTM32F429xx
CFLAGS  += -Wl,--gc-sections
#CFLAGS += -ffunction-sections -fdata-sections

# processor-specific flags
CFLAGS  += -mlittle-endian -mcpu=cortex-m4  -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -u _printf_float -u _scanf_float

#Path to STM32F4 FW .zip
STM32_FW = /home/branjb/.ac6/SW4STM32/firmwares/STM32Cube_FW_F4_V1.21.0

# library flags
#CFLAGS += --specs=nano.specs
CFLAGS  += -I$(STM32_FW)/Drivers/CMSIS/Device/ST/STM32F4xx/Include
CFLAGS  += -I$(STM32_FW)/Drivers/CMSIS/Include
CFLAGS  += -I/usr/lib/gcc/arm-none-eabi/6.3.1/include
CFLAGS  += -Iinc

# linker flags
CFLAGS  += -Wl,-T,LinkerScript.ld
LDFLAGS += -L/usr/lib/gcc/arm-none-eabi/6.3.1/lib/armv6-m -lc -lm

all: $(BIN_IMAGE)

$(BIN_IMAGE): $(EXECUTABLE)
	$(OBJCOPY) -O binary $^ $@

$(EXECUTABLE): src/*.c stm/*.c stm/*.s 
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
	
# "make install" to flash the firmware to the target STM32F0 chip
install:
	openocd -f config_openocd_stm32f0.cfg -c "stm_flash $(BIN_IMAGE)"

# "make debug_server" to establish a connector to the st-link v2 programmer/debugger
# this server will stay running until manually closed with Ctrl-C
debug_server:
	openocd -f config_openocd_stm32f0.cfg

# "make debug_nemiver" to debug using the Nemiver GUI.
# "make debug_server" must be called first and left running while using Nemiver.
debug_nemiver:
	arm-none-eabi-gdb --batch --command=config_gdb.cfg $(EXECUTABLE)
	nemiver --remote=localhost:3333 --gdb-binary=/home/farrellf/stm32/gcc-arm-none-eabi-4_8-2013q4/bin/arm-none-eabi-gdb $(EXECUTABLE)

# "make debug_cli" to debug using the arm-none-eabi-gdb command line tool.
# "make debug_server" must be called first and left running while using arm-none-eabi-gdb.
debug_cli:
	arm-none-eabi-gdb --silent command=config_gdb.cfg firmware.elf

clean:
	rm -rf $(EXECUTABLE)
	rm -rf $(BIN_IMAGE)

.PHONY: all clean debug_server debug_nemivier debug_cli
