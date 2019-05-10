#######################################################

TARGET = StateLight

#######################################################

# toolchain
CC = sdcc
OBJCOPY = objcopy
PACK_HEX = packihx
WCHISP = wchisptool

#######################################################


# Adjust the XRAM location and size to leave space for the USB DMA buffers
# Buffer layout in XRAM:
# 0x0000 Ep0Buffer[8]
# 0x0040 Ep1Buffer[8]
# 0x0080 EP2Buffer[2*64]
#
# This takes a total of 256bytes, so there are 768 bytes left.
XRAM_SIZE = 0x0300
XRAM_LOC = 0x0100

C_FILES = \
	../bitbang.c \
	../hardware.c \
	../logic.c \
	../main.c \
	../parser.c \
	../eeprom.c \
	../timer.c \
	../usb-cdc.c \
	../framework/include/debug.c

pre-flash:


ifndef FREQ_SYS
FREQ_SYS = 16000000
endif

ifndef XRAM_SIZE
XRAM_SIZE = 0x0400
endif

ifndef XRAM_LOC
XRAM_LOC = 0x0000
endif

ifndef CODE_SIZE
CODE_SIZE = 0x2800
endif

ROOT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CFLAGS := -V -mmcs51 --model-small \
	--xram-size $(XRAM_SIZE) --xram-loc $(XRAM_LOC) \
	--code-size $(CODE_SIZE) \
	-I$(ROOT_DIR)../framework/include -DFREQ_SYS=$(FREQ_SYS) \
	$(EXTRA_FLAGS)

LFLAGS := $(CFLAGS)

RELS := $(C_FILES:.c=.rel)

print-%  : ; @echo $* = $($*)

%.rel : %.c
	$(CC) -c $(CFLAGS) $<


# Note: SDCC will dump all of the temporary files into this one, so strip the paths from RELS
# For now, get around this by stripping the paths off of the RELS list.

$(TARGET).ihx: $(RELS)
	$(CC) $(notdir $(RELS)) $(LFLAGS) -o $(TARGET).ihx


$(TARGET).hex: $(TARGET).ihx
	$(PACK_HEX) $(TARGET).ihx > $(TARGET).hex


$(TARGET).bin: $(TARGET).ihx
	$(OBJCOPY) -I ihex -O binary $(TARGET).ihx $(TARGET).bin
	echo "Programm size"
	du $(TARGET).bin -b


flash: $(TARGET).bin pre-flash
	$(WCHISP) -f $(TARGET).bin -g


.DEFAULT_GOAL := all
all: $(TARGET).bin $(TARGET).hex


clean:
	rm -f \
	$(notdir $(RELS:.rel=.asm)) \
	$(notdir $(RELS:.rel=.lst)) \
	$(notdir $(RELS:.rel=.mem)) \
	$(notdir $(RELS:.rel=.rel)) \
	$(notdir $(RELS:.rel=.rst)) \
	$(notdir $(RELS:.rel=.sym)) \
	$(notdir $(RELS:.rel=.adb)) \
	$(TARGET).lk \
	$(TARGET).map \
	$(TARGET).mem \
	$(TARGET).ihx \
	$(TARGET).hex \
	$(TARGET).bin



## Download framework
framework:
	git clone https://github.com/Blinkinlabs/ch554_sdcc.git framework












