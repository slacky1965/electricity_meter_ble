
COMPILE_PREFIX = C:/TelinkSDK/opt/tc32/bin/tc32

AS      = $(COMPILE_PREFIX)-elf-as
CC      = $(COMPILE_PREFIX)-elf-gcc
LD      = $(COMPILE_PREFIX)-elf-ld
NM      = $(COMPILE_PREFIX)-elf-nm
OBJCOPY = $(COMPILE_PREFIX)-elf-objcopy
OBJDUMP = $(COMPILE_PREFIX)-elf-objdump
ARCH	= $(COMPILE_PREFIX)-elf-ar
SIZE	= $(COMPILE_PREFIX)-elf-size

TEL_CHIP := -DCHIP_TYPE=CHIP_TYPE_825x

# Select link lib
LIBS := -llt_825x -llt_general_stack

SDK_PATH := ./SDK

# Set Project Name
PROJECT_NAME := electricity_meter

# Set Electricity Meter Type
EMETER := KASKAD_1_MT
#EMETER := KASKAD_11
ELECTRICITY_TYPE := 'ELECTRICITY_TYPE=$(EMETER)'

# Set the serial port number for downloading the firmware
DOWNLOAD_PORT := COM4

SRC_PATH := ./src
OUT_PATH := ./out
UTILS_PATH := ./utils
MAKE_INCLUDES := ./make
VERSION := V$(shell awk -F " " '/VERSION/ {gsub("0x",""); printf "%.1f", $$3/10.0}' $(SRC_PATH)/include/app_config.h)

TL_Check = $(UTILS_PATH)/tl_check_fw.py

ifneq ($(SDK_PATH)/drivers/8258/gpio_8258.c, $(wildcard $(SDK_PATH)/drivers/8258/gpio_8258.c))
$(error "Please check if the SDK_PATH is correct, please check SDK_PATH is the setting correct！！！")
endif

OBJ_SRCS := 
S_SRCS := 
ASM_SRCS := 
C_SRCS := 
S_UPPER_SRCS := 
O_SRCS := 
FLASH_IMAGE := 
ELFS := 
OBJS := 
LST := 
SIZEDUMMY := 
OUT_DIR :=

GCC_FLAGS := \
-ffunction-sections \
-fdata-sections \
-Wall \
-O2 \
-fpack-struct \
-fshort-enums \
-finline-small-functions \
-std=gnu99 \
-fshort-wchar \
-fms-extensions \
-D$(ELECTRICITY_TYPE)

#INCLUDE_PATHS := -I$(SDK_PATH) -I$(PROJECT_PATH)/include
INCLUDE_PATHS := -I$(SDK_PATH) -I$(SDK_PATH)/drivers/8258 -I$(SRC_PATH)/include

GCC_FLAGS += $(TEL_CHIP)

#LS_FLAGS := $(PROJECT_PATH)/boot.link
LS_FLAGS := $(SDK_PATH)/boot/boot_32k_retn_8253_8258.link
#LS_FLAGS := $(SDK_PATH)/boot.link

#include SDK makefile
-include $(MAKE_INCLUDES)/common.mk
-include $(MAKE_INCLUDES)/vendor_common.mk
-include $(MAKE_INCLUDES)/application.mk
-include $(MAKE_INCLUDES)/drivers_8258.mk
-include $(MAKE_INCLUDES)/project.mk
-include $(MAKE_INCLUDES)/div_mod.mk
-include $(MAKE_INCLUDES)/boot.mk

# Add inputs and outputs from these tool invocations to the build variables 
LST_FILE := $(OUT_PATH)/$(PROJECT_NAME).lst
BIN_FILE := $(OUT_PATH)/$(PROJECT_NAME).bin
ELF_FILE := $(OUT_PATH)/$(PROJECT_NAME).elf

SIZEDUMMY := sizedummy

# All Target
all: clean pre-build main-build

flash: $(BIN_FILE)
	@python3 $(UTILS_PATH)/TlsrComProg.py -p$(DOWNLOAD_PORT) -f $(UTILS_PATH)/floader.bin we 0 $(BIN_FILE)
#	@python3 $(UTILS_PATH)/TlsrPgm.py -p$(DOWNLOAD_PORT) -t50 -a2550 -m -w we 0 $(BIN_FILE)

reset:
	@python3 $(UTILS_PATH)/TlsrPgm.py -p$(DOWNLOAD_PORT) -t50 -a2550 -m -w i

stop:
	@python3 $(UTILS_PATH)/TlsrPgm.py -p$(DOWNLOAD_PORT) -t50 -a2550 i

go:
	@python3 $(UTILS_PATH)/TlsrPgm.py -p$(DOWNLOAD_PORT) -w -m
	
erase-flash:
	@python3 $(UTILS_PATH)/TlsrComProg.py -p$(DOWNLOAD_PORT) -f $(UTILS_PATH)/floader.bin ea
#	@python3 $(UTILS_PATH)/TlsrPgm.py -p$(DOWNLOAD_PORT) -t50 -a2550 ea
	

# Main-build Target
main-build: $(ELF_FILE) secondary-outputs

# Tool invocations
$(ELF_FILE): $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: TC32 C Linker'
	@$(LD) --gc-sections -L $(SDK_PATH)/proj_lib -L $(OUT_PATH) -T $(LS_FLAGS) -o $(ELF_FILE) $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

$(LST_FILE): $(ELF_FILE)
	@echo 'Invoking: TC32 Create Extended Listing'
	@$(OBJDUMP) -x -D -l -S  $(ELF_FILE)  > $(LST_FILE)
	@echo 'Finished building: $@'
	@echo ' '

$(BIN_FILE): $(ELF_FILE)
	@echo 'Create Flash image (binary format)'
	@$(OBJCOPY) -v -O binary $(ELF_FILE)  $(BIN_FILE)
	@python3 $(TL_Check) $(BIN_FILE)
	@echo 'Finished building: $@'
	@echo ' '
	@cp $(BIN_FILE) $(PROJECT_NAME)_$(EMETER)_$(VERSION).bin

sizedummy: $(ELF_FILE)
	@echo 'Invoking: Print Size'
	@$(SIZE) -t $(ELF_FILE)
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE) $(ELFS) $(OBJS) $(LST) $(SIZEDUMMY) $(ELF_FILE) $(BIN_FILE) $(LST_FILE)
	-@echo ' '

pre-build:
	mkdir -p $(foreach s,$(OUT_DIR),$(OUT_PATH)$(s))
	-@echo ' '

secondary-outputs: $(BIN_FILE) $(LST_FILE) $(SIZEDUMMY)

.PHONY: all clean flash pre-build
.SECONDARY: main-build
