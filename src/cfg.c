#include <stdint.h>
#include "drivers\8258\flash.h"
#include "tl_common.h"

#include "cfg.h"
#include "log.h"
#include "bthome.h"
#include "ble.h"
#include "device.h"

_attribute_data_retention_ config_t config;
_attribute_data_retention_ static uint8_t default_config = false;
_attribute_data_retention_ uint8_t save_config = false;

_attribute_ram_code_ static void clear_user_data() {
    uint32_t flash_addr = BEGIN_USER_DATA;

    while(flash_addr < END_USER_DATA) {
        flash_erase_sector(flash_addr);
        flash_addr += FLASH_SECTOR_SIZE;
    }
}

static void init_default_config() {

    memset(&config, 0, sizeof(config_t));
    config.size = sizeof(config_t);
    config.id = ID_CONFIG;
    config.flash_addr = BEGIN_USER_DATA;
    config.measurement_period = MEASUREMENT_PERIOD;
    meter.division_factor = 1;
    set_device_type(device_kaskad_1_mt);
    default_config = true;
}

_attribute_ram_code_ void init_config() {
    config_t config_curr, config_next;
    uint32_t flash_addr = BEGIN_USER_DATA;
    uint8_t find_config = false;

    flash_read_page(flash_addr, sizeof(config_t), (uint8_t*)&config_curr);

    if (config_curr.id != ID_CONFIG) {
#if UART_PRINT_DEBUG_ENABLE
        printf("No saved config! Init.\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
        clear_user_data();
        init_default_config();
        write_config();
        return;
    }

    flash_addr += FLASH_PAGE_SIZE;

    while(flash_addr < END_USER_DATA) {
        flash_read_page(flash_addr, sizeof(config_t), (uint8_t*)&config_next);

        if (config_next.id == ID_CONFIG) {
            if ((config_curr.top + 1) == config_next.top ||
                    (config_curr.top == TOP_MASK && config_next.top == 0)) {
                memcpy(&config_curr, &config_next, sizeof(config_t));
                flash_addr += FLASH_PAGE_SIZE;
                continue;
            }
            find_config = true;
            break;
        }
        find_config = true;
        break;
    }

    if (find_config) {
        if (config_curr.size != sizeof(config_t)) {
#if UART_PRINT_DEBUG_ENABLE
            printf("Check new format config! Reinit.\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
            clear_user_data();
            if (config_curr.save_data.id == ID_DATA) {
                /* save old count in new config */
#if UART_PRINT_DEBUG_ENABLE
                printf("Find old saving data.\r\n");
                printf("Device address  - %05u\r\n", config_curr.save_data.address_device);
                if (config_curr.save_data.encrypted) {
                    printf("Encryption is enabled\r\n");
                    printf("Encryption key: 0x");
                    for (int i = 0; i < sizeof(config_curr.save_data.bindkey); i++) {
                        printf("%02x", config_curr.save_data.bindkey[i]);
                    }
                    printf("\r\n");
                } else {
                    printf("Encryption is disabled\r\n");
                }
#endif /* UART_PRINT_DEBUG_ENABLE */

                init_default_config();

                config.save_data.address_device = config_curr.save_data.address_device;
                config.save_data.encrypted = config_curr.save_data.encrypted;
                if (config.save_data.encrypted) {
                    memcpy(config.save_data.bindkey, config_curr.save_data.bindkey, sizeof(config.save_data.bindkey));
                }
                write_config();
            } else {
#if UART_PRINT_DEBUG_ENABLE
                printf("Can't find old electricity meter data. New data = 0.\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
                init_default_config();
                write_config();
            }
        } else {
            memcpy(&config, &config_curr, sizeof(config_t));
            config.flash_addr = flash_addr-FLASH_PAGE_SIZE;
#if UART_PRINT_DEBUG_ENABLE
            printf("Read config from flash address - 0x%x\r\n", config.flash_addr);
#endif /* UART_PRINT_DEBUG_ENABLE */
        }
    } else {
#if UART_PRINT_DEBUG_ENABLE
        printf("No active saved config! Reinit.\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
        clear_user_data();
        init_default_config();
        write_config();
    }
}

_attribute_ram_code_ void write_config() {
    if (default_config) {
        flash_erase_sector(config.flash_addr);
        flash_write_page(config.flash_addr, sizeof(config_t), (uint8_t*)&(config));
        default_config = false;
    } else {
        config.flash_addr += FLASH_PAGE_SIZE;
        if (config.flash_addr == END_USER_DATA) {
            config.flash_addr = BEGIN_USER_DATA;
        }
        if (config.flash_addr % FLASH_SECTOR_SIZE == 0) {
            flash_erase_sector(config.flash_addr);
        }
        config.top++;
        config.top &= TOP_MASK;
        flash_write_page(config.flash_addr, sizeof(config_t), (uint8_t*)&(config));
    }
    save_config = false;
#if UART_PRINT_DEBUG_ENABLE
    printf("Save config to flash address - 0x%x\r\n", config.flash_addr);
#endif /* UART_PRINT_DEBUG_ENABLE */
}

_attribute_ram_code_ void clear_config() {

    memset(&meter, 0, sizeof(meter_t));
    memset(&config.save_data, 0, sizeof(config.save_data));
    config.save_data.id = ID_DATA;
    config.measurement_period = MEASUREMENT_PERIOD;
    write_config();
    tariff_changed = true;
    pv_changed = true;
}

