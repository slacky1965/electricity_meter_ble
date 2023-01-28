
#OUT_DIR += /src

OBJS += $(OUT_PATH)/div_mod.o

# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/%.o: $(SDK_PATH)/%.S
	@echo 'Building file: $<'
	@$(CC) $(BOOT_FLAG) -c -o"$@" "$<"
