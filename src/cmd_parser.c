#include <stdint.h>
#include "tl_common.h"
#include "stack/ble/ble.h"

#include "cmd_parser.h"
#include "cfg.h"
#include "log.h"
#include "ble.h"
#include "app.h"
#include "device.h"

void cmd_parser(void * p) {
	rf_packet_att_data_t *req = (rf_packet_att_data_t*)p;
	uint8_t *in_data = req->dat;
	uint8_t len = req->l2cap-3;

    if (*in_data == CMD_SET_MEASUR_PERIOD && len == 2) {
        main_notify.measurement_period = config.measurement_period = in_data[1];
        write_config();
#if UART_PRINT_DEBUG_ENABLE
        printf("New measurement_period - %u %s\r\n", config.measurement_period, (config.measurement_period==1)?"minute":"minutes");
#endif /* UART_PRINT_DEBUG_ENABLE */
	} else if (*in_data == CMD_CLEAR_WHIYELIST && len == 1) {
#if UART_PRINT_DEBUG_ENABLE
	    printf("Reset whitelist\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
	    bls_smp_eraseAllParingInformation();
	    ev_adv_timeout(0,0,0);
//	    bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN);
    } else if (*in_data == CMD_RESET && len == 1) {
#if UART_PRINT_DEBUG_ENABLE
        printf("Reset is enabled after disconnection\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
        ble_connected |= conn_delayed_reset;
    } else if (*in_data == CMD_MAIN_NOTIFY) {
        main_notify.id = ELECTRICITYMETER_ID;
        main_notify.measurement_period = config.measurement_period;
        main_notify.version = VERSION;
        main_notify.encrypted = config.save_data.encrypted;
        main_notify.address = config.save_data.address_device;
        main_notify.device_type = config.save_data.device_type;
        mn_notify = NOTIFY_MAX;
        ble_send_main();

        if (config.save_data.encrypted) {
            bindkey_notify.id = BINDKEY_NOTIFY_ID;
            memcpy(bindkey_notify.bindkey, config.save_data.bindkey, sizeof(config.save_data.bindkey));
            bndk_notify = NOTIFY_MAX;
        }
        send_log_enable = true;
        lg_notify = NOTIFY_MAX;

        memset(&serial_number_notify, 0, sizeof(serial_number_notify_t));
        serial_number_notify.id = SERIAL_NUMBER_ID;
        /* check of serial number */
        if (meter.serial_number_len == 0) {
            meter.get_serial_number_data();
        }
        if (meter.serial_number_len) {
            memcpy(serial_number_notify.serial_number,
                   meter.serial_number,
                   meter.serial_number_len > sizeof(serial_number_notify.serial_number)?
                   sizeof(serial_number_notify.serial_number):meter.serial_number_len);
        }
        sn_notify = NOTIFY_MAX;

        memset(&date_release_notify, 0, sizeof(date_release_notify_t));
        date_release_notify.id = DATE_RELEASE_ID;
        /* check date of release */
        if (meter.date_release_len == 0) {
            meter.get_date_release_data();
        }
        if (meter.date_release_len) {
            memcpy(date_release_notify.date_release,
                   meter.date_release,
                   meter.date_release_len > sizeof(date_release_notify.date_release)?
                   sizeof(date_release_notify.date_release):meter.date_release_len);
        }
        dr_notify = NOTIFY_MAX;
#if UART_PRINT_DEBUG_ENABLE
        printf("Main notify start\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
    } else if (*in_data == CMD_SET_BINDKEY && len == 17) {
        in_data++;
        memcpy(config.save_data.bindkey, in_data, sizeof(config.save_data.bindkey));
        memcpy(bindkey_notify.bindkey, config.save_data.bindkey, sizeof(config.save_data.bindkey));
        if (config.save_data.encrypted == device_info_encrypt_none) {
            config.save_data.encrypted = device_info_encrypt;
            main_notify.encrypted = config.save_data.encrypted;
            bthome_beacon_init();
        }
        write_config();
        tariff_changed = pv_changed = true;
#if UART_PRINT_DEBUG_ENABLE
        printf("Set new bindkey - \"");
        for(int i = 0; i < sizeof(config.save_data.bindkey); i++) {
            printf("%02x", config.save_data.bindkey[i]);
        }
        printf("\"\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
    } else if (*in_data == CMD_RESET_BINDKEY) {
        config.save_data.encrypted = device_info_encrypt_none;
        memset(config.save_data.bindkey, 0, sizeof(config.save_data.bindkey));
        memset(bindkey_notify.bindkey, 0, sizeof(bindkey_notify.bindkey));
        write_config();
        tariff_changed = pv_changed = true;
#if UART_PRINT_DEBUG_ENABLE
        printf("Reset BindKey\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
    } else if (*in_data == CMD_SET_ADDRESS && len > 1) {
        len--;
        config.save_data.address_device = 0;
        if (len) config.save_data.address_device |= (in_data[len--] & 0xFF);
        if (len) config.save_data.address_device |= ((in_data[len--] << 8) & 0xFF00);
        if (len) config.save_data.address_device |= ((in_data[len--] << 16) & 0xFF0000);
        if (len) config.save_data.address_device |= ((in_data[len--] << 24) & 0xFF000000);
        write_config();
        meter.measure_meter();
        memset(&serial_number_notify, 0, sizeof(serial_number_notify_t));
        serial_number_notify.id = SERIAL_NUMBER_ID;
        /* check of serial number */
        if (meter.serial_number_len == 0) {
            meter.get_serial_number_data();
        }
        if (meter.serial_number_len) {
            memcpy(serial_number_notify.serial_number,
                   meter.serial_number,
                   meter.serial_number_len > sizeof(serial_number_notify.serial_number)?
                   sizeof(serial_number_notify.serial_number):meter.serial_number_len);
        }
        sn_notify = NOTIFY_MAX;

        memset(&date_release_notify, 0, sizeof(date_release_notify_t));
        date_release_notify.id = DATE_RELEASE_ID;
        /* check date of release */
        if (meter.date_release_len == 0) {
            meter.get_date_release_data();
        }
        if (meter.date_release_len) {
            memcpy(date_release_notify.date_release,
                   meter.date_release,
                   meter.date_release_len > sizeof(date_release_notify.date_release)?
                   sizeof(date_release_notify.date_release):meter.date_release_len);
        }
        dr_notify = NOTIFY_MAX;
#if UART_PRINT_DEBUG_ENABLE
        printf("New device address: %u\r\n", config.save_data.address_device);
#endif /* UART_PRINT_DEBUG_ENABLE */
    } else if (*in_data == CMD_CLEAR_CFG) {
        clear_config();
        main_notify.id = ELECTRICITYMETER_ID;
        main_notify.measurement_period = config.measurement_period;
        main_notify.address = config.save_data.address_device;
        mn_notify = NOTIFY_MAX;
        ble_send_main();
        tariff1_notify = NOTIFY_MAX;
        tariff2_notify = NOTIFY_MAX;
        tariff3_notify = NOTIFY_MAX;
        power_notify   = NOTIFY_MAX;
        voltage_notify = NOTIFY_MAX;
        memset(serial_number_notify.serial_number, 0, sizeof(serial_number_notify.serial_number));
        sn_notify      = NOTIFY_MAX;
        memset(date_release_notify.date_release, 0, sizeof(date_release_notify.date_release));
        dr_notify      = NOTIFY_MAX;
        config.save_data.encrypted = device_info_encrypt_none;
        memset(config.save_data.bindkey, 0, sizeof(config.save_data.bindkey));
        memset(bindkey_notify.bindkey, 0, sizeof(bindkey_notify.bindkey));
        write_config();
        tariff_changed = pv_changed = true;
#if UART_PRINT_DEBUG_ENABLE
        printf("Clear config (measurement data)\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
    } else {
#if UART_PRINT_DEBUG_ENABLE
        printf("Unknown or incomplete command 0x%X\r\n", *in_data);
#endif /* UART_PRINT_DEBUG_ENABLE */

    }
}
