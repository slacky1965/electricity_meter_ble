#include <stdint.h>
#include "tl_common.h"
#include "stack/ble/ble.h"

#include "device.h"
#include "kaskad_11.h"
#include "cfg.h"
#include "app_uart.h"
#include "app.h"

#if (!ELECTRICITY_TYPE)
#define ELECTRICITY_TYPE  KASKAD_11
#endif

#if (ELECTRICITY_TYPE == KASKAD_11)

#define LEVEL_READ 0x02
#define MIN_PKT_SIZE 0x06

_attribute_data_retention_ static k11_package_t k11_request_pkt;
_attribute_data_retention_ static k11_package_t k11_response_pkt;
_attribute_data_retention_ static uint8_t   def_password[] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
_attribute_data_retention_ static uint8_t   phases3;


_attribute_ram_code_ static uint8_t k11_checksum(const uint8_t *src_buffer) {
    uint8_t crc = 0;
    uint8_t len = src_buffer[0]-1;

    for (uint8_t i = 0; i < len; i++) {
        crc += src_buffer[i];
    }

    return crc;
}

_attribute_ram_code_ static uint8_t k11_send_command(k11_package_t *pkt) {

    size_t len;

    /* three attempts to write to uart */
    for (uint8_t attempt = 0; attempt < 3; attempt++) {
        len = write_bytes_to_uart((uint8_t*)pkt, pkt->len);
        if (len == pkt->len) {
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            printf("send bytes: %u\r\n", len);
#endif
            break;
        } else {
            len = 0;
        }
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("Attempt to send data to uart: %u\r\n", attempt+1);
#endif
        sleep_ms(250);
    }

    if (len == 0) {
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("Can't send a request pkt\r\n");
#endif
    } else {
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("request pkt: 0x");
        for (int i = 0; i < len; i++) {
            printf("%02x", ((uint8_t*)pkt)[i]);
        }
        printf("\r\n");
#endif
    }

    return len;
}

_attribute_ram_code_ static pkt_error_t k11_response_meter(uint8_t command) {

    size_t load_size, load_len = 0, len = PKT_BUFF_MAX_LEN;
    uint8_t ch, complete = false;;
    pkt_error_no = PKT_ERR_TIMEOUT;
    uint8_t *buff = (uint8_t*)&k11_response_pkt;

    memset(buff, 0, sizeof(k11_package_t));

    for (uint8_t attempt = 0; attempt < 3; attempt ++) {
        load_size = 0;
        while (available_buff_uart() && load_size < PKT_BUFF_MAX_LEN) {
            ch = read_byte_from_buff_uart();
            if (load_size == 0) {
                if (ch == 0) {
                    load_len++;
                    continue;
                } else if (get_queue_len_buff_uart() < ch-1) {
                    load_len++;
                    continue;
                }
                len = ch;
            }

            load_len++;
            buff[load_size++] = ch;
            len--;

            if (len == 0) {
                if (load_size < MIN_PKT_SIZE) {
                    len = PKT_BUFF_MAX_LEN;
                    load_size = 0;
                    pkt_error_no = PKT_ERR_INCOMPLETE;
                    continue;
                }

                if (load_size == buff[0]) {
                    attempt = 3;
                    pkt_error_no = PKT_OK;
                    complete = true;
                    break;
                }
            }

        }
        sleep_ms(250);
    }

#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
    printf("read bytes: %u\r\n", load_len);
#endif

    if (load_size) {
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("response pkt: 0x");
        for (int i = 0; i < load_size; i++) {
            printf("%02x", buff[i]);
        }
        printf("\r\n");
#endif

        if (complete) {
            uint8_t crc = k11_checksum(buff);
            if (crc == buff[k11_response_pkt.len-1]) {
                if (buff[k11_response_pkt.len-2] == 0x01) {
                    if (k11_response_pkt.address == config.save_data.address_device) {
                        if (k11_response_pkt.cmd == command) {
                            pkt_error_no = PKT_OK;
                        } else {
                            pkt_error_no = PKT_ERR_DIFFERENT_COMMAND;
                        }
                    } else {
                        pkt_error_no = PKT_ERR_ADDRESS;
                    }
                } else {
                    pkt_error_no = PKT_ERR_RESPONSE;
                }
            } else {
                pkt_error_no = PKT_ERR_CRC;
            }
        } else {
            pkt_error_no = PKT_ERR_INCOMPLETE;
        }
    }

#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
    switch (pkt_error_no) {
        case PKT_ERR_TIMEOUT:
            printf("Response timed out\r\n");
            break;
        case PKT_ERR_RESPONSE:
            printf("Response error\r\n");
            break;
        case PKT_ERR_UNKNOWN_FORMAT:
        case PKT_ERR_NO_PKT:
            printf("Unknown response format\r\n");
            break;
        case PKT_ERR_DIFFERENT_COMMAND:
            printf("Request and response command are different\r\n");
            break;
        case PKT_ERR_INCOMPLETE:
            printf("Not a complete response\r\n");
            break;
        case PKT_ERR_UNSTUFFING:
            printf("Wrong unstuffing\r\n");
            break;
        case PKT_ERR_ADDRESS:
            printf("Invalid device address\r\n");
            break;
        case PKT_ERR_CRC:
            printf("Wrong CRC\r\n");
            break;
        case PKT_ERR_UART:
            printf("UART is busy\r\n");
            break;
        default:
            break;
    }
#endif

    if (pkt_error_no != PKT_OK && get_queue_len_buff_uart()) {
        k11_response_meter(command);
    }

    return pkt_error_no;
}

_attribute_ram_code_ static void k11_set_header(uint8_t cmd) {

    memset(&k11_request_pkt, 0, sizeof(k11_package_t));

    k11_request_pkt.len = 1;
    k11_request_pkt.cmd = cmd;
    k11_request_pkt.len++;
    k11_request_pkt.address = config.save_data.address_device;
    k11_request_pkt.len += 2;
}

_attribute_ram_code_ static uint8_t k11_open_channel() {

    uint8_t pos = 0;

    k11_set_header(cmd_k11_open_channel);

    k11_request_pkt.data[pos++] = LEVEL_READ;

    memcpy(k11_request_pkt.data+pos, def_password, sizeof(def_password));
    pos += sizeof(def_password);
    k11_request_pkt.len += pos+1;
    uint8_t crc = k11_checksum((uint8_t*)&k11_request_pkt);
    k11_request_pkt.data[pos] = crc;

    if (k11_send_command(&k11_request_pkt)) {
        if (k11_response_meter(cmd_k11_open_channel) == PKT_OK) {
            return true;
        }
    }

    return false;
}

_attribute_ram_code_ static void k11_close_channel() {

    k11_set_header(cmd_k11_close_channel);

    k11_request_pkt.len++;
    uint8_t crc = k11_checksum((uint8_t*)&k11_request_pkt);
    k11_request_pkt.data[0] = crc;

    if (k11_send_command(&k11_request_pkt)) {
        k11_response_meter(cmd_k11_close_channel);
    }
}

_attribute_ram_code_ static void k11_set_tariff_num(uint8_t tariff_num) {

    k11_set_header(cmd_k11_tariffs_data);

    k11_request_pkt.data[0] = tariff_num;
    k11_request_pkt.len += 2;
    k11_request_pkt.data[1] = k11_checksum((uint8_t*)&k11_request_pkt);
}

_attribute_ram_code_ static void k11_get_tariffs_data() {

    for (uint8_t tariff_num = 1; tariff_num < 4; tariff_num++) {
        k11_set_tariff_num(tariff_num);
        if (k11_send_command(&k11_request_pkt)) {
            if (k11_response_meter(cmd_k11_tariffs_data) == PKT_OK) {

                k11_pkt_tariff_t *pkt_tariff = (k11_pkt_tariff_t*)&k11_response_pkt;

                switch (tariff_num) {
                    case 1:
                        if (pkt_tariff->value < meter.tariff_1) {
                            meter.tariff_1 = pkt_tariff->value;
                            tariff_changed = true;
                            tariff1_notify = NOTIFY_MAX;
                        }
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
                        printf("tariff1: %u,%u\r\n", meter.tariff_1 / divisor(meter.division_factor),
                                                     meter.tariff_1 % divisor(meter.division_factor));
#endif
                        break;
                    case 2:
                        if (pkt_tariff->value < meter.tariff_2) {
                            meter.tariff_2 = pkt_tariff->value;
                            tariff_changed = true;
                            tariff1_notify = NOTIFY_MAX;
                        }
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
                        printf("tariff2: %u,%u\r\n", meter.tariff_2 / divisor(meter.division_factor),
                                                     meter.tariff_2 % divisor(meter.division_factor));
#endif
                        break;
                    case 3:
                        if (pkt_tariff->value < meter.tariff_3) {
                            meter.tariff_3 = pkt_tariff->value;
                            tariff_changed = true;
                            tariff1_notify = NOTIFY_MAX;
                        }
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
                        printf("tariff3: %u,%u\r\n", meter.tariff_3 / divisor(meter.division_factor),
                                                     meter.tariff_3 % divisor(meter.division_factor));
#endif
                        break;
                    default:
                        break;
                }
            }
        }
    }


}

_attribute_ram_code_ static void k11_set_net_parameters(uint8_t param) {

    k11_set_header(cmd_k11_net_parameters);

    k11_request_pkt.data[0] = param;
    k11_request_pkt.len += 2;
    k11_request_pkt.data[1] = k11_checksum((uint8_t*)&k11_request_pkt);

}

_attribute_ram_code_ static void k11_get_voltage_data() {

    k11_set_net_parameters(net_voltage);

    if (k11_send_command(&k11_request_pkt)) {
        if (k11_response_meter(cmd_k11_net_parameters) == PKT_OK) {
            k11_pkt_voltage_t *pkt_voltage = (k11_pkt_voltage_t*)&k11_response_pkt;
            if (meter.voltage != pkt_voltage->voltage) {
                meter.voltage = pkt_voltage->voltage;
                pv_changed = true;
                voltage_notify = NOTIFY_MAX;
            }
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            printf("volts: %u,%02u\r\n", pkt_voltage->voltage / divisor(meter.division_factor),
                                         pkt_voltage->voltage % divisor(meter.division_factor));
#endif

        }
    }
}

_attribute_ram_code_ static void k11_get_power_data() {

    k11_set_net_parameters(net_power);

    if (k11_send_command(&k11_request_pkt)) {
        if (k11_response_meter(cmd_k11_net_parameters) == PKT_OK) {
            k11_pkt_power_t *pkt_power = (k11_pkt_power_t*)&k11_response_pkt;
            uint32_t power = from24to32(pkt_power->power);
            if (meter.power != power) {
                meter.power = power;
                pv_changed = true;
                power_notify = NOTIFY_MAX;
            }

#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            printf("power: %u,%02u\r\n", power / divisor(meter.division_factor),
                                         power % divisor(meter.division_factor));
#endif
        }
    }
}


_attribute_ram_code_ static void k11_get_amps_data() {

    k11_set_net_parameters(net_amps);

    if (k11_send_command(&k11_request_pkt)) {
        if (k11_response_meter(cmd_k11_net_parameters) == PKT_OK) {
            /* check 1 phase or 3 phases */
            if (k11_response_pkt.len > 9) {
                phases3 = true;
            } else {
                phases3 = false;
                k11_pkt_amps_t *pkt_amps = (k11_pkt_amps_t*)&k11_response_pkt;
    #if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
                printf("amps: %u,%02u\r\n", pkt_amps->amps/1000, pkt_amps->amps%1000);
    #endif
            }
        }
    }
}

_attribute_ram_code_ void k11_get_date_release_data() {

    k11_pkt_release_t *pkt;

    k11_set_header(cmd_k11_date_release);

    k11_request_pkt.len++;
    uint8_t crc = k11_checksum((uint8_t*)&k11_request_pkt);
    k11_request_pkt.data[0] = crc;

    if (k11_send_command(&k11_request_pkt)) {
        if (k11_response_meter(cmd_k11_open_channel) == PKT_OK) {
            pkt = (k11_pkt_release_t*)&k11_response_pkt;
            meter.date_release_len = sprintf((char*)meter.date_release, "%02u.%02u.%u", pkt->day, pkt->month, pkt->year+2000);
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            printf("Date of release: %s\r\n", meter.date_release_len);
#endif
        }
    }
}

_attribute_ram_code_ uint8_t k11_get_serial_number_data() {

    k11_set_header(cmd_k11_serial_number);

    k11_request_pkt.len++;
    uint8_t crc = k11_checksum((uint8_t*)&k11_request_pkt);
    k11_request_pkt.data[0] = crc;

    if (k11_send_command(&k11_request_pkt)) {
        if (k11_response_meter(cmd_k11_serial_number) == PKT_OK) {
            meter.serial_number_len = k11_response_pkt.len - MIN_PKT_SIZE;
            memcpy(meter.serial_number, k11_response_pkt.data, meter.serial_number_len);
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            printf("Serial Number: %s\r\n", meter.serial_number);
#endif
            return true;
        }
    }

    return false;
}

_attribute_ram_code_ void k11_measure_meter() {

    if (k11_open_channel()) {

        if (new_start) {
            k11_get_amps_data();                /* get amps and check phases num */
            k11_get_serial_number_data();
            k11_get_date_release_data();
            new_start = false;
        }

        if (phases3) {
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            printf("Sorry, three-phase meter!\r\n");
#endif
        } else {
            k11_get_tariffs_data();             /* get 3 tariffs        */
            k11_get_voltage_data();             /* get voltage net ~220 */
            k11_get_power_data();               /* get power            */
        }
        k11_close_channel();
    }
}



#endif

