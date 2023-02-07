#include <stdint.h>
#include "tl_common.h"
#include "stack/ble/ble.h"

#include "device.h"
#include "cfg.h"
#include "app_uart.h"
#include "app.h"

#if (ELECTRICITY_TYPE == KASKAD_1_MT)

#define START       0x73
#define BOUNDARY    0x55
#define PROG_ADDR   0xffff
#define PASSWORD    0x00000000
#define PARAMS_LEN  0x20
#define STUFF_55    0x11
#define STUFF_73    0x22

_attribute_data_retention_ static package_t request_pkt;
_attribute_data_retention_ static package_t response_pkt;
_attribute_data_retention_ static uint8_t   package_buff[PKT_BUFF_MAX_LEN];
_attribute_data_retention_ static uint8_t   first_start = true;
_attribute_data_retention_ static pkt_error_t pkt_error_no;

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

_attribute_ram_code_ static uint32_t from24to32(const uint8_t *str) {

    uint32_t value;

    value = str[0] & 0xff;
    value |= (str[1] << 8) & 0xff00;
    value |= (str[2] << 16) & 0xff0000;

    return value;
}

_attribute_ram_code_ static uint16_t divisor(const uint8_t division_factor) {

    switch (division_factor & 0x03) {
        case 0: return 1;
        case 1: return 10;
        case 2: return 100;
        case 3: return 1000;
    }

    return 1;
}

_attribute_ram_code_ static void set_command(command_t command) {

    memset(&request_pkt, 0, sizeof(package_t));

    request_pkt.start = START;
    request_pkt.boundary = BOUNDARY;
    request_pkt.header.params_len = 0b00100000; // 0x20 to device
    request_pkt.header.address_to = config.meter.address; // = 20109;
    request_pkt.header.address_from = PROG_ADDR;
    request_pkt.header.command = command & 0xff;
    request_pkt.header.password_status = PASSWORD;

    switch (command) {
        case cmd_open_channel:
        case cmd_current_data:
        case cmd_power_data:
            request_pkt.pkt_len = 2 + sizeof(package_header_t) + 2;
            request_pkt.data[0] = checksum((uint8_t*)&request_pkt, request_pkt.pkt_len);
            request_pkt.data[1] = BOUNDARY;
            break;
            break;
        case cmd_amps_data:
        case cmd_volts_data:
        case cmd_serial_number:
        case cmd_date_release:
        case cmd_factory_manufacturer:
        case cmd_name_device:
        case cmd_name_device2:
            request_pkt.header.params_len |= 0x01;
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
            uint8_t *data = (uint8_t*)&request_pkt;
            printf("request pkt: 0x");
            for (int i = 0; i < len; i++) {
                printf("%02x", data[i]);
            }
            printf("\r\n");
#endif
    }
}

_attribute_ram_code_ pkt_error_t response_meter(command_t command) {

    size_t len, load_size = 0;
    uint8_t ch, complete = false;

    pkt_error_no = PKT_ERR_TIMEOUT;

    memset(package_buff, 0, sizeof(package_buff));

    for (uint8_t attempt = 0; attempt < 3; attempt ++) {
        load_size = 0;
        while (available_buff_uart() && load_size <= PKT_BUFF_MAX_LEN) {

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
        if (complete) {
            len = byte_unstuffing(load_size);
            if (len) {
                response_pkt.pkt_len = len;
                uint8_t crc = checksum((uint8_t*)&response_pkt, response_pkt.pkt_len);

                if (crc == response_pkt.data[(response_pkt.header.params_len & 0x1f)]) {
                    if (response_pkt.header.address_from == config.meter.address) {
                        if (response_pkt.header.command == (command & 0xff)) {
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
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        uint8_t *data = (uint8_t*)&response_pkt;
        printf("package power: 0x");
        for (int i = 0; i < response_pkt.pkt_len; i++) {
            printf("%02x", data[i]);
        }
        printf("\r\n");
#endif

            return &response_pkt;
        }
    }
    return NULL;
}

_attribute_ram_code_ uint8_t ping_start_data() {

    package_t *pkt;

    pkt = get_pkt_data(cmd_open_channel);

    if (pkt) {
        return true;
    }

    return false;
}

_attribute_ram_code_ void get_tariffs_data() {

    tariffs_meter_data_t *tariffs_response;
    package_t            *pkt;

    pkt = get_pkt_data(cmd_current_data);

    if (pkt) {
        tariffs_response = (tariffs_meter_data_t*)pkt->data;

        if (config.meter.tariff_1 != (tariffs_response->tariff_1)) {
            config.meter.tariff_1 = tariffs_response->tariff_1;
            tariff_changed = true;
            tariff1_notify = NOTIFY_MAX;
            save_config = true;
        }

        if (config.meter.tariff_2 != (tariffs_response->tariff_2)) {
            config.meter.tariff_2 = tariffs_response->tariff_2;
            tariff_changed = true;
            tariff2_notify = NOTIFY_MAX;
            save_config = true;
        }

        if (config.meter.tariff_3 != (tariffs_response->tariff_3)) {
            config.meter.tariff_3 = tariffs_response->tariff_3;
            tariff_changed = true;
            tariff3_notify = NOTIFY_MAX;
            save_config = true;
        }

        if (config.meter.division_factor != (tariffs_response->byte_cfg & 0x03)) {
            config.meter.division_factor = tariffs_response->byte_cfg & 0x03;
            save_config = true;
        }

#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("tariff1: %u,%u\r\n", config.meter.tariff_1 / divisor(config.meter.division_factor),
                                     config.meter.tariff_1 % divisor(config.meter.division_factor));
        printf("tariff2: %u,%u\r\n", config.meter.tariff_2 / divisor(config.meter.division_factor),
                                     config.meter.tariff_2 % divisor(config.meter.division_factor));
        printf("tariff3: %u,%u\r\n", config.meter.tariff_3 / divisor(config.meter.division_factor),
                                     config.meter.tariff_3 % divisor(config.meter.division_factor));
#endif

    }
}

_attribute_ram_code_ void get_amps_data() {

    amps_meter_data_t *amps_response;
    package_t         *pkt;
    uint32_t amps;

    pkt = get_pkt_data(cmd_amps_data);

    if (pkt) {
        amps_response = (amps_meter_data_t*)pkt->data;
        amps = from24to32(amps_response->amps);
        if ((pkt->header.params_len & 0x1f) == 3) {
            amps &= 0xffff;
        }
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("phase: %u, amps: %u,%02u\r\n", amps_response->phase_num, amps/1000, amps%1000);
#endif
    }
}

_attribute_ram_code_ void get_voltage_data() {

    volts_meter_data_t *volts_response;
    package_t          *pkt;

    pkt = get_pkt_data(cmd_volts_data);

    if (pkt) {
        volts_response = (volts_meter_data_t*)pkt->data;
        if (config.meter.voltage != volts_response->volts) {
            config.meter.voltage = volts_response->volts;
            pv_changed = true;
            voltage_notify = NOTIFY_MAX;
            save_config = true;
        }

#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("phase: %u, volts: %u,%02u\r\n", volts_response->phase_num, volts_response->volts/100, volts_response->volts%100);
#endif

    }
}

_attribute_ram_code_ void get_power_data() {

    power_meter_data_t *power_response;
    package_t          *pkt;
    uint32_t            power;

    pkt = get_pkt_data(cmd_power_data);

    if (pkt) {
        power_response = (power_meter_data_t*)pkt->data;
        power = from24to32(power_response->power);
        if (config.meter.power != power) {
            config.meter.power = power;
            pv_changed = true;
            power_notify = NOTIFY_MAX;
            save_config = true;
        }

        if (config.meter.division_factor != (power_response->byte_cfg & 0x03)) {
            config.meter.division_factor = power_response->byte_cfg & 0x03;
            save_config = true;
        }

#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("power: %u,%02u\r\n", power / divisor(config.meter.division_factor),
                                     power % divisor(config.meter.division_factor));
#endif
    }
}

_attribute_ram_code_ void get_serial_number_data() {
    data31_meter_data_t *serial_number_response;
    package_t           *pkt;

    pkt = get_pkt_data(cmd_serial_number);

    if (pkt) {
        serial_number_response = (data31_meter_data_t*)pkt;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("Serial Number: ");
        for (int i = 0; i < 31; i++) {
            if (serial_number_response->data[i] != 0) {
                printf("%c", serial_number_response->data[i]);
                config.meter.serial_number[i] = serial_number_response->data[i];
                config.meter.sn_len++;
            }
            else break;
        }
        printf("\r\n");
#else
        for (int i = 0; i < 31; i++) {
            if (serial_number_response->data[i] != 0) {
                config.meter.serial_number[i] = serial_number_response->data[i];
                config.meter.sn_len++;
            }
            else break;
        }
#endif
    }

}

_attribute_ram_code_ void get_date_release_data() {
    data31_meter_data_t *date_release_response;
    package_t           *pkt;

    pkt = get_pkt_data(cmd_date_release);

    if (pkt) {
        date_release_response = (data31_meter_data_t*)pkt;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("Date of release: ");
        for (int i = 0; i < 31; i++) {
            if (date_release_response->data[i] != 0) {
                printf("%c", date_release_response->data[i]);
                config.meter.date_release[i] = date_release_response->data[i];
                config.meter.dr_len++;
            }
            else break;
        }
        printf("\r\n");
#else
        for (int i = 0; i < 31; i++) {
            if (date_release_response->data[i] != 0) {
                printf("%c", date_release_response->data[i]);
                config.meter.date_release[i] = date_release_response->data[i];
                config.meter.dr_len++;
            }
            else break;
        }
#endif
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

_attribute_ram_code_ void measure_meter() {

    if (ping_start_data()) {
        get_tariffs_data();
        get_voltage_data();
        get_power_data();
        if (first_start) {
            get_amps_data();
            get_serial_number_data();
            get_date_release_data();
            first_start = false;
        }
    }
    if (save_config) {
        write_config();
    }
}

#endif

