# Add inputs and outputs from these tool invocations to the build variables 
OUT_DIR += \
/boot/8251 \
/boot/8253 \
/boot/8258

OBJS += \
$(OUT_PATH)/boot/8251/cstartup_8251_RET_16K.o \
$(OUT_PATH)/boot/8251/cstartup_8251_RET_32K.o \
$(OUT_PATH)/boot/8253/cstartup_8253_RET_16K.o \
$(OUT_PATH)/boot/8253/cstartup_8253_RET_32K.o \
$(OUT_PATH)/boot/8258/cstartup_8258_RET_16K.o \
$(OUT_PATH)/boot/8258/cstartup_8258_RET_32K.o \

BOOT_FLAG := -DMCU_STARTUP_825X -DMCU_STARTUP_8253_RET_32K

# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/boot/%.o: $(SDK_PATH)/boot/%.S
	@echo 'Building file: $<'
	@$(CC) $(BOOT_FLAG) -c -o"$@" "$<"

