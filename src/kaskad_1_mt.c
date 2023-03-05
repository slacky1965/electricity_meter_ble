#include <stdint.h>
#include "tl_common.h"
#include "stack/ble/ble.h"

#include "device.h"
#include "kaskad_1_mt.h"
#include "cfg.h"
#include "app_uart.h"
#include "app.h"
#include "ble.h"

#define START       0x73
#define BOUNDARY    0x55
#define PROG_ADDR   0xffff
#define PASSWORD    0x00000000
#define PARAMS_LEN  0x20
#define STUFF_55    0x11
#define STUFF_73    0x22

_attribute_data_retention_ static package_t request_pkt;
_attribute_data_retention_ static package_t response_pkt;
_attribute_data_retention_ static uint8_t        package_buff[PKT_BUFF_MAX_LEN];

_attribute_ram_code_ static uint8_t checksum(const uint8_t *src_buffer, uint8_t len) {
  // skip 73 55 header (and 55 footer is beyond checksum anyway)
  const uint8_t* table = &src_buffer[2];
  const uint8_t packet_len = len - 4;

  const uint8_t generator = 0xA9;

  uint8_t crc = 0;
  for(const uint8_t* ptr = table; ptr < table + packet_len; ptr++){
    crc ^= *ptr;
    for (uint8_t bit = 8; bit > 0; bit--)
      if (crc & 0x80)
        crc = (crc << 1) ^ generator;
      else
        crc <<= 1;
  }

  return crc;
}

_attribute_ram_code_ static void set_command(command_t command) {

    memset(&request_pkt, 0, sizeof(package_t));

    request_pkt.start = START;
    request_pkt.boundary = BOUNDARY;
    request_pkt.header.from_to = 1; // to device
    request_pkt.header.address_to = config.save_data.address_device; // = 20109;
    request_pkt.header.address_from = PROG_ADDR;
    request_pkt.header.command = command & 0xff;
    request_pkt.header.password_status = PASSWORD;

    switch (command) {
        case cmd_open_channel:
        case cmd_tariffs_data:
        case cmd_power_data:
        case cmd_read_configure:
        case cmd_get_info:
        case cmd_test_error:
        case cmd_resource_battery:
            request_pkt.pkt_len = 2 + sizeof(package_header_t) + 2;
            request_pkt.data[0] = checksum((uint8_t*)&request_pkt, request_pkt.pkt_len);
            request_pkt.data[1] = BOUNDARY;
            break;
        case cmd_amps_data:
        case cmd_volts_data:
        case cmd_serial_number:
        case cmd_date_release:
        case cmd_factory_manufacturer:
        case cmd_name_device:
        case cmd_name_device2:
            request_pkt.header.data_len = 1;
            request_pkt.pkt_len = 2 + sizeof(package_header_t) + 3;
            request_pkt.data[0] = (command >> 8) & 0xff;   // sub command
            request_pkt.data[1] = checksum((uint8_t*)&request_pkt, request_pkt.pkt_len);
            request_pkt.data[2] = BOUNDARY;
            break;
        default:
            break;
    }
}

_attribute_ram_code_ static size_t byte_stuffing() {

    uint8_t *source, *receiver;
    size_t len = 0;

    source = (uint8_t*)&request_pkt;
    receiver = package_buff;

    *(receiver++) = *(source++);
    len++;
    *(receiver++) = *(source++);
    len++;

    for (int i = 0; i < (request_pkt.pkt_len-3); i++) {
        if (*source == BOUNDARY) {
            *(receiver++) = START;
            len++;
            *receiver = STUFF_55;
        } else if (*source == START) {
            *(receiver++) = START;
            len++;
            *receiver = STUFF_73;
        } else {
            *receiver = *source;
        }
        source++;
        receiver++;
        len++;
    }

    *(receiver) = *(source);
    len++;

    return len;
}

_attribute_ram_code_ static size_t byte_unstuffing(uint8_t load_len) {

    size_t   len = 0;
    uint8_t *source = package_buff;
    uint8_t *receiver = (uint8_t*)&response_pkt;

    *(receiver++) = *(source++);
    len++;
    *(receiver++) = *(source++);
    len++;

    for (int i = 0; i < (load_len-3); i++) {
        if (*source == START) {
            source++;
            len--;
            if (*source == STUFF_55) {
                *receiver = BOUNDARY;
            } else if (*source == STUFF_73) {
                *receiver = START;
            } else {
                /* error */
                return 0;
            }
        } else {
            *receiver = *source;
        }
        source++;
        receiver++;
        len++;
    }

    *(receiver) = *(source);
    len++;

    return len;
}

_attribute_ram_code_ static void send_command(command_t command) {

    uint8_t buff_len, len = 0;

    set_command(command);

    buff_len = byte_stuffing();

    /* three attempts to write to uart */
    for (uint8_t attempt = 0; attempt < 3; attempt++) {
        len = write_bytes_to_uart(package_buff, buff_len);
        if (len == buff_len) {
            request_pkt.load_len = len;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("send bytes: %u\r\n", len);
#endif
            break;
        } else {
            request_pkt.load_len = 0;
        }
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("Attempt to send data to uart: %u\r\n", attempt+1);
#endif
        sleep_ms(250);
    }

    if (request_pkt.load_len == 0) {
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("Can't send a request pkt\r\n");
#endif
    } else {
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            printf("request pkt: 0x");
            for (int i = 0; i < len; i++) {
                printf("%02x", ((uint8_t*)&request_pkt)[i]);
            }
            printf("\r\n");
#endif
    }
}

_attribute_ram_code_ static pkt_error_t response_meter(command_t command) {

    size_t len, load_size = 0;
    uint8_t err = 0, ch, complete = false;

    pkt_error_no = PKT_ERR_TIMEOUT;

    memset(package_buff, 0, sizeof(package_buff));

    for (uint8_t attempt = 0; attempt < 3; attempt ++) {
        load_size = 0;
        while (available_buff_uart() && load_size < PKT_BUFF_MAX_LEN) {

            ch = read_byte_from_buff_uart();

            if (load_size == 0) {
                if (ch != START) {
                    pkt_error_no = PKT_ERR_NO_PKT;
                    continue;
                }
            } else if (load_size == 1) {
                if (ch != BOUNDARY) {
                    load_size = 0;
                    pkt_error_no = PKT_ERR_UNKNOWN_FORMAT;
                    continue;
                }
            } else if (ch == BOUNDARY) {
                complete = true;
            }

            package_buff[load_size++] = ch;

            if (complete) {
                attempt = 3;
                pkt_error_no = PKT_OK;
                break;
            }
        }
        sleep_ms(250);
    }

#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
    printf("read bytes: %u\r\n", load_size);
#endif

    if (load_size) {
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("response pkt: 0x");
        for (int i = 0; i < load_size; i++) {
            printf("%02x", package_buff[i]);
        }
        printf("\r\n");
#endif
        if (complete) {
            len = byte_unstuffing(load_size);
            if (len) {
                response_pkt.pkt_len = len;
                uint8_t crc = checksum((uint8_t*)&response_pkt, response_pkt.pkt_len);
                if (crc == response_pkt.data[(response_pkt.header.data_len)]) {
                    response_status_t *status = (response_status_t*)&response_pkt.header.password_status;
                    if (status->error == PKT_OK) {
                        if (response_pkt.header.address_from == config.save_data.address_device) {
                            if (response_pkt.header.command == (command & 0xff)) {
                                pkt_error_no = PKT_OK;
                            } else {
                                pkt_error_no = PKT_ERR_DIFFERENT_COMMAND;
                            }
                        } else {
                            pkt_error_no = PKT_ERR_ADDRESS;
                        }
                    } else {
                        pkt_error_no = PKT_ERR_RESPONSE;
                        err = status->error;
                    }
                } else {
                    pkt_error_no = PKT_ERR_CRC;
                }
            } else {
                pkt_error_no = PKT_ERR_UNSTUFFING;
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
            printf("Response error: 0x%02x\r\n", err);
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

_attribute_ram_code_ static package_t *get_pkt_data(command_t command) {

    send_command(command);
    sleep_ms(200);
    if (request_pkt.load_len > 0) {
        if (response_meter(command) == PKT_OK) {
            return &response_pkt;
        }
    }
    return NULL;
}

_attribute_ram_code_ static uint8_t ping_start_data() {

    package_t *pkt;

    pkt = get_pkt_data(cmd_open_channel);

    if (pkt) {
        return true;
    }

    return false;
}

_attribute_ram_code_ static void get_tariffs_data() {

    pkt_tariffs_t *tariffs_response;
    package_t            *pkt;

    pkt = get_pkt_data(cmd_tariffs_data);

    if (pkt) {
        tariffs_response = (pkt_tariffs_t*)pkt->data;

        if (meter.tariff_1 < tariffs_response->tariff_1) {
            meter.tariff_1 = tariffs_response->tariff_1;
            tariff_changed = true;
            tariff1_notify = NOTIFY_MAX;
        }

        if (meter.tariff_2 < tariffs_response->tariff_2) {
            meter.tariff_2 = tariffs_response->tariff_2;
            tariff_changed = true;
            tariff2_notify = NOTIFY_MAX;
        }

        if (meter.tariff_3 < tariffs_response->tariff_3) {
            meter.tariff_3 = tariffs_response->tariff_3;
            tariff_changed = true;
            tariff3_notify = NOTIFY_MAX;
        }

#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("tariff1: %u,%u\r\n", meter.tariff_1 / divisor(meter.division_factor),
                                     meter.tariff_1 % divisor(meter.division_factor));
        printf("tariff2: %u,%u\r\n", meter.tariff_2 / divisor(meter.division_factor),
                                     meter.tariff_2 % divisor(meter.division_factor));
        printf("tariff3: %u,%u\r\n", meter.tariff_3 / divisor(meter.division_factor),
                                     meter.tariff_3 % divisor(meter.division_factor));
#endif

    }
}

_attribute_ram_code_ static void get_amps_data() {

    pkt_amps_t *amps_response;
    package_t  *pkt;
    uint32_t amps;

    pkt = get_pkt_data(cmd_amps_data);

    if (pkt) {
        amps_response = (pkt_amps_t*)pkt->data;
        amps = from24to32(amps_response->amps);
        /* pkt->header.params_len & 0x1f == 3 -> amps 2 bytes
         * pkt->header.params_len & 0x1f == 4 -> amps 3 bytes
         */
        if (pkt->header.data_len == 3) {
            amps &= 0xffff;
        }
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("phase: %u, amps: %u,%02u\r\n", amps_response->phase_num, amps/1000, amps%1000);
#endif
    }
}

_attribute_ram_code_ static void get_voltage_data() {

    pkt_volts_t *volts_response;
    package_t   *pkt;

    pkt = get_pkt_data(cmd_volts_data);

    if (pkt) {
        volts_response = (pkt_volts_t*)pkt->data;
        if (meter.voltage != volts_response->volts) {
            meter.voltage = volts_response->volts;
            pv_changed = true;
            voltage_notify = NOTIFY_MAX;
        }

#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("phase: %u, volts: %u,%02u\r\n", volts_response->phase_num,
                                                volts_response->volts / divisor(meter.division_factor),
                                                volts_response->volts % divisor(meter.division_factor));
#endif

    }
}

_attribute_ram_code_ static void get_power_data() {

    pkt_power_t *power_response;
    package_t   *pkt;
    uint32_t         power;

    pkt = get_pkt_data(cmd_power_data);

    if (pkt) {
        power_response = (pkt_power_t*)pkt->data;
        power = from24to32(power_response->power);
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

_attribute_ram_code_ uint8_t get_serial_number_data_kaskad1mt() {
    pkt_data31_t *serial_number_response;
    package_t    *pkt;

    pkt = get_pkt_data(cmd_serial_number);

    if (pkt) {
        serial_number_response = (pkt_data31_t*)pkt;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("Serial Number: %s\r\n", serial_number_response->data);
#endif

        if (memcmp(meter.serial_number, serial_number_response->data, DATA_MAX_LEN) != 0) {
            meter.serial_number_len = sprintf((char*)meter.serial_number, "%s", serial_number_response->data);
            memcpy(serial_number_notify.serial_number, meter.serial_number,
                   meter.serial_number_len > sizeof(serial_number_notify.serial_number)?
                   sizeof(serial_number_notify.serial_number):meter.serial_number_len);
            sn_notify = NOTIFY_MAX;
        }
        return true;
    }

    return false;
}

_attribute_ram_code_ void get_date_release_data_kaskad1mt() {

    pkt_data31_t *date_release_response;
    package_t    *pkt;

    pkt = get_pkt_data(cmd_date_release);

    if (pkt) {
        date_release_response = (pkt_data31_t*)pkt;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("Date of release: %s", date_release_response->data);
#endif

        if (memcpy(meter.date_release, date_release_response->data, DATA_MAX_LEN) != 0) {
            meter.date_release_len = sprintf((char*)meter.date_release, "%s", date_release_response->data);
            memcpy(date_release_notify.date_release, meter.date_release,
                   meter.date_release_len > sizeof(date_release_notify.date_release)?
                   sizeof(date_release_notify.date_release):meter.date_release_len);
            dr_notify = NOTIFY_MAX;
        }
    }

}

_attribute_ram_code_ static void get_configure_data() {

    pkt_read_cfg_t *read_cfg;
    package_t      *pkt;

    pkt = get_pkt_data(cmd_read_configure);

    if (pkt) {
        read_cfg = (pkt_read_cfg_t*)pkt->data;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("divisor: %u\r\n", divisor(read_cfg->divisor));
#endif
        if (meter.division_factor != read_cfg->divisor) {
            meter.division_factor = read_cfg->divisor;
        }
    }
}

_attribute_ram_code_ static void get_resbat_data() {

    pkt_resbat_t *resbat;
    package_t    *pkt;

    pkt = get_pkt_data(cmd_resource_battery);

    if (pkt) {
        resbat = (pkt_resbat_t*)pkt->data;

#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("Resource battery: %u.%u\r\n", (resbat->worktime*100)/resbat->lifetime,
                                             ((resbat->worktime*100)%resbat->lifetime)*100/resbat->lifetime);
#endif
        uint8_t battery_level = (resbat->worktime*100)/resbat->lifetime;

        if (((resbat->worktime*100)%resbat->lifetime) >= (resbat->lifetime/2)) {
            battery_level++;
        }

        if (meter.battery_level != battery_level) {
            meter.battery_level = battery_level;
            pv_changed = true;
        }

    }
}



void pkt_test(command_t command) {
    package_t *pkt;
    pkt = get_pkt_data(command);

    if (pkt) {
    } else {
        printf("pkt = NULL\r\n");
    }
}

_attribute_ram_code_ void measure_meter_kaskad1mt() {

    if (ping_start_data()) {           /* ping to device       */
        if (new_start) {               /* after reset          */
            get_configure_data();      /* get divisor          */
            get_serial_number_data_kaskad1mt();
            get_date_release_data_kaskad1mt();
            new_start = false;
        }
        get_resbat_data();             /* get resource battery */
        get_tariffs_data();            /* get 3 tariffs        */
        get_voltage_data();            /* get voltage net ~220 */
        get_power_data();              /* get power            */
        get_amps_data();               /* get amps             */
    }
}

