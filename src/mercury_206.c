#include <stdint.h>
#include "tl_common.h"
#include "stack/ble/ble.h"

#include "device.h"
#include "mercury_206.h"
#include "cfg.h"
#include "app_uart.h"
#include "app.h"

_attribute_data_retention_ static m206_package_t m206_request_pkt;
_attribute_data_retention_ static m206_package_t m206_response_pkt;

_attribute_ram_code_ static uint16_t m206_checksum(const uint8_t *src_buffer, uint8_t len) {

    const uint16_t generator = 0xa001;

    uint16_t crc = 0xffff;

    for (const uint8_t *ptr = src_buffer; ptr < src_buffer + len; ptr++) {
        crc ^= *ptr;

        for (uint8_t bit = 8; bit > 0; bit--) {
            if (crc & 1)
                crc = (crc >> 1) ^ generator;
            else
                crc >>= 1;
        }
    }

    return crc;
}

_attribute_ram_code_ static uint32_t reverse32(uint32_t in) {
    uint32_t out;
    uint8_t *source = (uint8_t*)&in;
    uint8_t *destination = (uint8_t*)&out;

    destination[3] = source[0];
    destination[2] = source[1];
    destination[1] = source[2];
    destination[0] = source[3];

    return out;
}

_attribute_ram_code_ static uint8_t from_bcd_to_dec(uint8_t bcd) {

    uint8_t dec = ((bcd >> 4) & 0x0f) * 10 + (bcd & 0x0f);

    return dec;
}

_attribute_ram_code_ static uint8_t m206_send_command(m206_package_t *pkt) {

    size_t len;

    /* three attempts to write to uart */
    for (uint8_t attempt = 0; attempt < 3; attempt++) {
        len = write_bytes_to_uart((uint8_t*)pkt, pkt->pkt_len);
        if (len == pkt->pkt_len) {
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

_attribute_ram_code_ static pkt_error_t m206_response_meter(uint8_t command) {

    uint8_t load_size, load_len = 0;
    pkt_error_no = PKT_ERR_TIMEOUT;
    uint8_t *buff = (uint8_t*)&m206_response_pkt;

    memset(buff, 0, sizeof(m206_package_t));

    for (uint8_t attempt = 0; attempt < 3; attempt ++) {
        load_size = 0;
        while(available_buff_uart() && load_size < PKT_BUFF_MAX_LEN) {
            buff[load_size++] = read_byte_from_buff_uart();
            load_len++;
        }

        if (load_size > 6) break;

        sleep_ms(250);
    }

#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
    printf("read bytes: %u\r\n", load_len);
#endif


    if (load_size) {
        if (load_size > 6) {
            m206_response_pkt.pkt_len = load_size;
            uint16_t crc = m206_checksum((uint8_t*)&m206_response_pkt, load_size-2);
            uint16_t crc_pkt = ((uint8_t*)&m206_response_pkt)[load_size-2] & 0xff;
            crc_pkt |= (((uint8_t*)&m206_response_pkt)[load_size-1] << 8) & 0xff00;
            if (crc == crc_pkt) {
                if (reverse32(m206_response_pkt.address) == config.save_data.address_device) {
                    if (m206_response_pkt.cmd == command) {
                        pkt_error_no = PKT_OK;
                    } else {
                        pkt_error_no = PKT_ERR_DIFFERENT_COMMAND;
                    }
                } else {
                    pkt_error_no = PKT_ERR_ADDRESS;
                }
            } else {
                pkt_error_no = PKT_ERR_CRC;
            }
        } else {
            pkt_error_no = PKT_ERR_RESPONSE;
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

    return pkt_error_no;
}

_attribute_ram_code_ static void m206_set_command(uint8_t cmd) {

    memset(&m206_request_pkt, 0, sizeof(m206_package_t));

    m206_request_pkt.address = reverse32(config.save_data.address_device);
    m206_request_pkt.cmd = cmd;
    m206_request_pkt.pkt_len = 5;
    uint16_t crc = m206_checksum((uint8_t*)&m206_request_pkt, m206_request_pkt.pkt_len);
    m206_request_pkt.data[0] = crc & 0xff;
    m206_request_pkt.data[1] = (crc >> 8) & 0xff;
    m206_request_pkt.pkt_len += 2;

}

_attribute_ram_code_ static uint32_t tariff_from_bcd(uint32_t tariff_bcd) {

    uint32_t tariff_dec = 0;
    uint8_t *p_tariff = (uint8_t*)&tariff_bcd;

    tariff_dec += from_bcd_to_dec(p_tariff[0]) * 1000000;
    tariff_dec += from_bcd_to_dec(p_tariff[1]) * 10000;
    tariff_dec += from_bcd_to_dec(p_tariff[2]) * 100;
    tariff_dec += from_bcd_to_dec(p_tariff[3]);

    return tariff_dec;
}

_attribute_ram_code_ static void m206_get_tariffs_data() {

    m206_set_command(cmd_m206_tariffs_data);

    if (m206_send_command(&m206_request_pkt)) {
        if (m206_response_meter(cmd_m206_tariffs_data) == PKT_OK) {
            m206_pkt_tariffs_t *pkt_tariffs = (m206_pkt_tariffs_t*)&m206_response_pkt;
            uint32_t tariff1 = tariff_from_bcd(pkt_tariffs->tariff_1);
            uint32_t tariff2 = tariff_from_bcd(pkt_tariffs->tariff_2);
            uint32_t tariff3 = tariff_from_bcd(pkt_tariffs->tariff_3);

            if (meter.tariff_1 < tariff1) {
                meter.tariff_1 = tariff1;
                tariff_changed = true;
                tariff1_notify = NOTIFY_MAX;
            }

            if (meter.tariff_2 < tariff2) {
                meter.tariff_2 = tariff2;
                tariff_changed = true;
                tariff1_notify = NOTIFY_MAX;
            }

            if (meter.tariff_3 < tariff3) {
                meter.tariff_3 = tariff3;
                tariff_changed = true;
                tariff1_notify = NOTIFY_MAX;
            }

#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            printf("tariff1: %u,%u\r\n", meter.tariff_1 / 10, meter.tariff_1 % 10);
            printf("tariff2: %u,%u\r\n", meter.tariff_2 / 10, meter.tariff_2 % 10);
            printf("tariff3: %u,%u\r\n", meter.tariff_3 / 10, meter.tariff_3 % 10);
#endif

        }
    }
}

_attribute_ram_code_ static void m206_get_net_params_data() {

    m206_set_command(cmd_m206_net_params);

    if (m206_send_command(&m206_request_pkt)) {
        if (m206_response_meter(cmd_m206_net_params) == PKT_OK) {
            m206_pkt_net_params_t *pkt_net_params = (m206_pkt_net_params_t*)&m206_response_pkt;
            uint32_t power = 0;
            power += from_bcd_to_dec(pkt_net_params->power[0]) * 10000;
            power += from_bcd_to_dec(pkt_net_params->power[1]) * 100;
            power += from_bcd_to_dec(pkt_net_params->power[2]);
            if (meter.power != power) {
                meter.power = power;
                pv_changed = true;
                power_notify = NOTIFY_MAX;
            }
            uint16_t volts = 0;
            uint8_t *volts_bcd = (uint8_t*)&pkt_net_params->volts;
            volts += from_bcd_to_dec(volts_bcd[0]) * 100;
            volts += from_bcd_to_dec(volts_bcd[1]);
            if (meter.voltage != volts) {
                meter.voltage = volts;
                pv_changed = true;
                power_notify = NOTIFY_MAX;
            }
            uint16_t amps = 0;
            uint8_t *amps_bcd = (uint8_t*)&pkt_net_params->amps;
            amps += from_bcd_to_dec(amps_bcd[0]) * 100;
            amps += from_bcd_to_dec(amps_bcd[1]);
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            printf("voltage: %u.%u\r\n", meter.voltage/10, meter.voltage%10);
            printf("power:   %u\r\n", meter.power);
            printf("amps:    %u.%u\r\n", amps/100, amps%100);
#endif
        }
    }
}

_attribute_ram_code_ static void m206_get_resbat_data() {

    m206_set_command(cmd_m206_running_time);

    if (m206_send_command(&m206_request_pkt)) {
        if (m206_response_meter(cmd_m206_running_time) == PKT_OK) {
            m206_pkt_running_time_t *pkt_running_time = (m206_pkt_running_time_t*)&m206_response_pkt;
            printf("tl: %02u%02u%02u\r\n", from_bcd_to_dec(pkt_running_time->tl[0]),
                                           from_bcd_to_dec(pkt_running_time->tl[1]),
                                           from_bcd_to_dec(pkt_running_time->tl[2]));
            printf("tlb: %02u%02u%02u\r\n",from_bcd_to_dec(pkt_running_time->tlb[0]),
                                           from_bcd_to_dec(pkt_running_time->tlb[1]),
                                           from_bcd_to_dec(pkt_running_time->tlb[2]));
        }
    }
}

_attribute_ram_code_ void m206_get_date_release_data() {

    m206_set_command(cmd_m206_date_release);

    if (m206_send_command(&m206_request_pkt)) {
        if (m206_response_meter(cmd_m206_date_release) == PKT_OK) {
            m206_pkt_release_t *pkt_release = (m206_pkt_release_t*)&m206_response_pkt;
            release_month = from_bcd_to_dec(pkt_release->date[1]);
            release_year = from_bcd_to_dec(pkt_release->date[2]);
            meter.date_release_len = sprintf((char*)meter.date_release, "%02u.%02u.20%02u",
                                             from_bcd_to_dec(pkt_release->date[0]),
                                             release_month, release_year);
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            printf("data release: %s\r\n", meter.date_release);
#endif
        }
    }
}


_attribute_ram_code_ uint8_t m206_get_serial_number_data() {

    m206_set_command(cmd_m206_serial_number);

    if (m206_send_command(&m206_request_pkt)) {
        if (m206_response_meter(cmd_m206_serial_number) == PKT_OK) {
            m206_pkt_serial_num_t *pkt_serial_num = (m206_pkt_serial_num_t*)&m206_response_pkt;
            meter.serial_number_len = sprintf((char*)meter.serial_number, "%u", pkt_serial_num->addr);
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            printf("serial number: %08x (%u)\r\n", pkt_serial_num->addr, pkt_serial_num->addr);
#endif
            return true;
        }
    }

    return false;
}

_attribute_ram_code_ void m206_measure_meter() {

    if (m206_get_serial_number_data()) {
        if (new_start) {
            m206_get_date_release_data();
            new_start = false;
        }
        m206_get_net_params_data();
        m206_get_tariffs_data();
        m206_get_resbat_data();
    }
}

