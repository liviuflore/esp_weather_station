# tnx to mamalala
# Changelog
# Changed the variables to include the header file directory
# Added global var for the XTENSA tool root
#
# Modified for test1-ESP8266 Project
#   Changed to use esptool-py to generate firmware images
#   Added TOOLKIT_DIR as a common base for all ESP related stuff
#   TOOLKIT_DIR must be set as enviroment variable before calling make
#
#
#
# Base projects directory, shoud be set in enviroment
#TOOLKIT_DIR	= D:\_code\esp\toolkit

# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= $(TOOLKIT_DIR)\xtensa-lx106-elf\bin

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(TOOLKIT_DIR)\esp_iot_rtos_sdk
#SDK_BASE	?= $(TOOLKIT_DIR)\esp_iot_sdk_1_2_0

#Esptool.py path and port
ESPTOOL		?= python $(TOOLKIT_DIR)\esptool-py.py
#ESPTOOL		?= python $(TOOLKIT_DIR)\esptool.py
ESPPORT		?= COM4

# name for the target project
TARGET		= $(PROJECT_NAME)_app

# Output directories to store intermediate compiled files
# relative to the project directory
BUILD_BASE	= build

# which modules (subdirectories) of the project to include in compiling
MODULES         = src src/www src/drivers
EXTRA_INCDIR    = include 

# libraries used in this project, mainly provided by the SDK
LIBS		=  gcc hal phy pp net80211 wpa main freertos lwip udhcp

# compiler flags using during compilation of source files
CFLAGS		= -Os -g -O2 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

# linker script used for the above linkier step
LD_SCRIPT	= eagle.app.v6.ld

# various paths from the SDK used in this project
SDK_LIBDIR	= lib 
SDK_LDDIR	= ld
SDK_INCDIR	= include include/espressif include/lwip include/lwip/ipv4 include/lwip/ipv6 extra_include

# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
OD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objdump



####
#### no user configurable options below here
####
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

SRC			:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ			:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC)) $(patsubst %.cpp,$(BUILD_BASE)/%.o,$(SRC))
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))

INCDIR		:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

VERBOSE=1
V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

vpath %.c $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
endef



.PHONY: all checkdirs flash clean

all: checkdirs $(TARGET_OUT) 

$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
	$(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@
	$(Q) $(OD) -h $@
	$(vecho) "FW $@"
	$(Q) $(ESPTOOL) elf2image $(TARGET_OUT) 

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

firmware:
	$(Q) mkdir -p $@

flash: 	$(TARGET_OUT)-0x00000.bin $(TARGET_OUT)-0x40000.bin
	-$(ESPTOOL) --port $(ESPPORT) write_flash 0x00000 $(TARGET_OUT)-0x00000.bin 0x40000 $(TARGET_OUT)-0x40000.bin

clean:
	$(Q) rm -rf $(BUILD_BASE)
	$(Q) rm -rf ./obj


$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
