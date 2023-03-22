OUT_DIR += /src

OBJS += \
$(OUT_PATH)/src/cfg.o \
$(OUT_PATH)/src/log.o \
$(OUT_PATH)/src/bthome.o \
$(OUT_PATH)/src/app_att.o \
$(OUT_PATH)/src/app_adc.o \
$(OUT_PATH)/src/cmd_parser.o \
$(OUT_PATH)/src/ble.o \
$(OUT_PATH)/src/ccm.o \
$(OUT_PATH)/src/app.o \
$(OUT_PATH)/src/app_uart.o \
$(OUT_PATH)/src/device.o \
$(OUT_PATH)/src/kaskad_1_mt.o \
$(OUT_PATH)/src/kaskad_11.o \
$(OUT_PATH)/src/mercury_206.o \
$(OUT_PATH)/src/energomera_ce102m.o \
$(OUT_PATH)/src/main.o

# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/src/%.o: $(SRC_PATH)/%.c
	@echo 'Building file: $<'
	@$(CC) $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"