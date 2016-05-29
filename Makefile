# filenames
EXECUTABLE=firmware.elf
BIN_IMAGE=firmware.bin

# toolchain
CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

# debugging and optimization flags
CFLAGS   = -g -O2 -std=c99 -Winline
#CFLAGS += -ffunction-sections -fdata-sections

# processor-specific flags
CFLAGS  += -mlittle-endian -mcpu=cortex-m0  -mthumb

# library flags
CFLAGS  += -I"../../STM32F0 CMSIS and StdPeriphLib/Libraries/STM32F0xx_StdPeriph_Driver/inc"
CFLAGS  += -I"../../STM32F0 CMSIS and StdPeriphLib/Libraries/CMSIS/Device/ST/STM32F0xx/Include"
CFLAGS  += -I"../../STM32F0 CMSIS and StdPeriphLib/Libraries/CMSIS/Include"
CFLAGS  += -I"../../gcc-arm-none-eabi-4.9_2015q3/arm-none-eabi/include"

# linker flags
CFLAGS  += -Wl,--gc-sections
CFLAGS  += -Wl,-T,stm/stm32f0.ld
LDFLAGS  = -L"../../STM32F0 CMSIS and StdPeriphLib/Libraries" -lstm32f0
LDFLAGS += -L"../../gcc-arm-none-eabi-4.9_2015q3/arm-none-eabi/lib/armv6-m" -lc -lm
#LDFLAGS += --specs=nano.specs
LDFLAGS += --specs=rdimon.specs -lrdimon # for semihosting

all: $(BIN_IMAGE)

$(BIN_IMAGE): $(EXECUTABLE)
	$(OBJCOPY) -O binary $^ $@

$(EXECUTABLE): *.c f0lib/*.c stm/system_stm32f0xx.c stm/startup_stm32f0xx.s
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
	arm-none-eabi-size $(EXECUTABLE)
	
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
	arm-none-eabi-gdb --silent --command=config_gdb.cfg firmware.elf

clean:
	rm -rf $(EXECUTABLE)
	rm -rf $(BIN_IMAGE)

.PHONY: all clean debug_server debug_nemivier debug_cli
