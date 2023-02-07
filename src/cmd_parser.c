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
        main_notify.encrypted = config.encrypted;
        main_notify.address = config.meter.address;
        mn_notify = NOTIFY_MAX;
        ble_send_main();

        if (config.encrypted) {
            bindkey_notify.id = BINDKEY_NOTIFY_ID;
            memcpy(bindkey_notify.bindkey, config.bindkey, sizeof(config.bindkey));
            bndk_notify = NOTIFY_MAX;
        }
        send_log_enable = true;
        lg_notify = NOTIFY_MAX;

        memset(&serial_number_notify, 0, sizeof(serial_number_notify_t));
        serial_number_notify.id = SERIAL_NUMNER_ID;
        /* check of serial number */
        if (config.meter.sn_len == 0) {
            get_serial_number_data();
        }
        if (config.meter.sn_len) {
            memcpy(serial_number_notify.serial_number,
                   config.meter.serial_number,
                   config.meter.sn_len > sizeof(serial_number_notify.serial_number)?
                   sizeof(serial_number_notify.serial_number):config.meter.sn_len);
        }
        sn_notify = NOTIFY_MAX;

        memset(&date_release_notify, 0, sizeof(date_release_notify_t));
        date_release_notify.id = DATE_RELEASE_ID;
        /* check date of release */
        if (config.meter.dr_len == 0) {
            get_date_release_data();
        }
        if (config.meter.dr_len) {
            memcpy(date_release_notify.date_release,
                   config.meter.date_release,
                   config.meter.dr_len > sizeof(date_release_notify.date_release)?
                   sizeof(date_release_notify.date_release):config.meter.dr_len);
        }
        dr_notify = NOTIFY_MAX;
#if UART_PRINT_DEBUG_ENABLE
        printf("Main notify start\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
    } else if (*in_data == CMD_SET_BINDKEY && len == 17) {
        in_data++;
        memcpy(config.bindkey, in_data, sizeof(config.bindkey));
        memcpy(bindkey_notify.bindkey, config.bindkey, sizeof(config.bindkey));
        if (config.encrypted == device_info_encrypt_none) {
            config.encrypted = device_info_encrypt;
            main_notify.encrypted = config.encrypted;
            bthome_beacon_init();
        }
        write_config();
        tariff_changed = pv_changed = true;
#if UART_PRINT_DEBUG_ENABLE
        printf("Set new bindkey - \"");
        for(int i = 0; i < 16; i++) {
            printf("%02x", config.bindkey[i]);
        }
        printf("\"\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
    } else if (*in_data == CMD_RESET_BINDKEY) {
        config.encrypted = device_info_encrypt_none;
        memset(config.bindkey, 0, sizeof(config.bindkey));
        memset(bindkey_notify.bindkey, 0, sizeof(bindkey_notify.bindkey));
        write_config();
        tariff_changed = pv_changed = true;
#if UART_PRINT_DEBUG_ENABLE
        printf("Reset BindKey\r\n");
#endif /* UART_PRINT_DEBUG_ENABLE */
    } else if (*in_data == CMD_SET_ADDRESS && len > 1) {
        len--;
        config.meter.address = 0;
        if (len) config.meter.address |= (in_data[len--] & 0xFF);
        if (len) config.meter.address |= ((in_data[len--] << 8) & 0xFF00);
        write_config();
        measure_meter();
        memset(&serial_number_notify, 0, sizeof(serial_number_notify_t));
        serial_number_notify.id = SERIAL_NUMNER_ID;
        /* check of serial number */
        if (config.meter.sn_len == 0) {
            get_serial_number_data();
        }
        if (config.meter.sn_len) {
            memcpy(serial_number_notify.serial_number,
                   config.meter.serial_number,
                   config.meter.sn_len > sizeof(serial_number_notify.serial_number)?
                   sizeof(serial_number_notify.serial_number):config.meter.sn_len);
        }
        sn_notify = NOTIFY_MAX;

        memset(&date_release_notify, 0, sizeof(date_release_notify_t));
        date_release_notify.id = DATE_RELEASE_ID;
        /* check date of release */
        if (config.meter.dr_len == 0) {
            get_date_release_data();
        }
        if (config.meter.dr_len) {
            memcpy(date_release_notify.date_release,
                   config.meter.date_release,
                   config.meter.dr_len > sizeof(date_release_notify.date_release)?
                   sizeof(date_release_notify.date_release):config.meter.dr_len);
        }
        dr_notify = NOTIFY_MAX;
#if UART_PRINT_DEBUG_ENABLE
        printf("New device address: %u\r\n", config.meter.address);
#endif /* UART_PRINT_DEBUG_ENABLE */
    } else if (*in_data == CMD_CLEAR_CFG) {
        clear_config();
        main_notify.id = ELECTRICITYMETER_ID;
        main_notify.measurement_period = config.measurement_period;
        main_notify.address = config.meter.address;
        mn_notify = NOTIFY_MAX;
        ble_send_main();
        tariff1_notify = NOTIFY_MAX;
        tariff2_notify = NOTIFY_MAX;
        tariff3_notify = NOTIFY_MAX;
        power_notify   = NOTIFY_MAX;
        voltage_notify = NOTIFY_MAX;
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
