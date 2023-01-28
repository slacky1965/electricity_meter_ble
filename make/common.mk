# Add inputs and outputs from these tool invocations to the build variables 
OUT_DIR += /common


OBJS += \
$(OUT_PATH)/common/string.o \
$(OUT_PATH)/common/utility.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/common/%.o: $(SDK_PATH)/common/%.c
	@echo 'Building file: $<'
	@$(CC) $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"