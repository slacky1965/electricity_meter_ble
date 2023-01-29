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
#define CONN_TYPE   0xffff
#define PASSWORD    0x00000000
#define ADD_LEN     0x20

_attribute_data_retention_ request_pkt_t    request_pkt;
_attribute_data_retention_ response_pkt_t   response_pkt;

static uint8_t checksum(const uint8_t *src_buffer, uint8_t len) {
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

static void set_command(cmd_t command) {

    memset(&request_pkt, 0, sizeof(request_pkt_t));

    request_pkt.head.start = START;
    request_pkt.head.boundary = BOUNDARY;
    request_pkt.head.add_len = ADD_LEN;
    request_pkt.head.nothing = 0;
    request_pkt.head.meter_address = config.meter.address; // = 20109;
    request_pkt.head.conn_type = CONN_TYPE;
    request_pkt.head.command = command;
    request_pkt.head.password = PASSWORD;

    switch (command) {
        case cmd_open_channel:
        case cmd_current_data:
        case cmd_power_data:
            request_pkt.pkt_len = sizeof(request_header_t) + 2;
            request_pkt.tail[0] = checksum((uint8_t*)&request_pkt, request_pkt.pkt_len);
            request_pkt.tail[1] = BOUNDARY;
            break;
        case cmd_amps_data:
        case cmd_volts_data:
            request_pkt.head.add_len |= 0x01;
            request_pkt.pkt_len = sizeof(request_header_t) + 3;
            request_pkt.tail[0] = 0x01;
            request_pkt.tail[1] = checksum((uint8_t*)&request_pkt, request_pkt.pkt_len);
            request_pkt.tail[2] = BOUNDARY;
            break;
        default:
            break;
    }


}

static void send_command(cmd_t command) {
    set_command(command);
    uint8_t len = send_to_uart((uint8_t*)&request_pkt, request_pkt.pkt_len);
    if (len == request_pkt.pkt_len) {
        request_pkt.load_len = len;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        uint8_t *data = (uint8_t*)&request_pkt;
        printf("request pkt: 0x");
        for (int i = 0; i < len; i++) {
            printf("%02x", data[i]);
        }
        printf("\r\n");
#endif
    } else {
        request_pkt.load_len = 0;
    }
}

static void response_meter(cmd_t command) {

    uint8_t buff[UART_DATA_LEN] = {0};
    size_t data_len, residue, len, load_size = 0;

    memset(&response_pkt, 0, sizeof(response_pkt));

    data_len = residue = get_data_len_from_uart();

    while (1) {
        len = response_from_uart(buff+load_size, residue);
        if (len == -1) {
            return;
        } else if (len == 0) {
            break;
        } else if (len < residue) {
            load_size += len;
            residue -= len;
        } else {
            load_size += len;
            break;
        }
    }

    if (load_size == data_len) {
        if (buff[0] == START && buff[1] == BOUNDARY && buff[data_len-1] == BOUNDARY && buff[8] == command) {
            memcpy(&response_pkt, buff, sizeof(response_header_t));
            len = response_pkt.head.data_len+2;
            memcpy(response_pkt.response_data, &buff[load_size-len], len);
            response_pkt.pkt_len = sizeof(response_header_t)+len;
        }
    }
}

uint8_t first_start_data() {

    first_meter_data_t  *first_reponse;
    uint8_t ret = false;

    send_command(cmd_open_channel);
    sleep_ms(200);
    if (request_pkt.pkt_len > 0) {
        response_meter(cmd_open_channel);
        if (response_pkt.pkt_len == sizeof(first_meter_data_t)) {
            first_reponse = (first_meter_data_t*)&response_pkt;
            if (first_reponse->addr == config.meter.address) {
                ret = true;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
                uint8_t *data = (uint8_t*)&response_pkt;
                printf("response first start: 0x");
                for (int i = 0; i < response_pkt.pkt_len; i++) {
                    printf("%02x", data[i]);
                }
                printf("\r\n");
#endif
            }
       }
    }

    return ret;
}

void get_current_data() {

    current_meter_data_t    *current_response;

    send_command(cmd_current_data);
    sleep_ms(200);
    if (request_pkt.pkt_len > 0) {
        response_meter(cmd_current_data);
        if (response_pkt.pkt_len == sizeof(current_meter_data_t)) {
            current_response = (current_meter_data_t*)&response_pkt;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            uint8_t *data = (uint8_t*)&response_pkt;
            printf("response current: 0x");
            for (int i = 0; i < response_pkt.pkt_len; i++) {
                printf("%02x", data[i]);
            }
            printf("\r\n");
#endif

            if (config.meter.tariff_1 != (current_response->tariff_1)) {
                config.meter.tariff_1 = current_response->tariff_1;
                tariff_changed = true;
                tariff1_notify = NOTIFY_MAX;
                save_config = true;
            }

            if (config.meter.tariff_2 != (current_response->tariff_2)) {
                config.meter.tariff_2 = current_response->tariff_2;
                tariff_changed = true;
                tariff2_notify = NOTIFY_MAX;
                save_config = true;
            }

            if (config.meter.tariff_3 != (current_response->tariff_3)) {
                config.meter.tariff_3 = current_response->tariff_3;
                tariff_changed = true;
                tariff3_notify = NOTIFY_MAX;
                save_config = true;
            }

#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            printf("tariff1: %u,%u\r\n", config.meter.tariff_1/100, config.meter.tariff_1%100);
            printf("tariff2: %u,%u\r\n", config.meter.tariff_2/100, config.meter.tariff_2%100);
            printf("tariff3: %u,%u\r\n", config.meter.tariff_3/100, config.meter.tariff_3%100);
#endif
        }
    }

}

void get_amps_data() {

    amps_meter_data_t   *amps_response;

    send_command(cmd_amps_data);
    sleep_ms(200);
    if (request_pkt.pkt_len > 0) {
        response_meter(cmd_amps_data);
        if (response_pkt.pkt_len == sizeof(amps_meter_data_t)) {
            amps_response = (amps_meter_data_t*)&response_pkt;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            uint8_t *data = (uint8_t*)&response_pkt;
            printf("response amps: 0x");
            for (int i = 0; i < response_pkt.pkt_len; i++) {
                printf("%02x", data[i]);
            }
            printf("\r\n");
            printf("amps: %u,%02u\r\n", amps_response->amps/1000, amps_response->amps%1000);
#endif
        }
    }

}

void get_voltage_data() {

    volts_meter_data_t  *volts_response;

    send_command(cmd_volts_data);
    sleep_ms(200);
    if (request_pkt.pkt_len > 0) {
        response_meter(cmd_volts_data);
        if (response_pkt.pkt_len == sizeof(volts_meter_data_t)) {
            volts_response = (volts_meter_data_t*)&response_pkt;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            uint8_t *data = (uint8_t*)&response_pkt;
            printf("response volts: 0x");
            for (int i = 0; i < response_pkt.pkt_len; i++) {
                printf("%02x", data[i]);
            }
            printf("\r\n");
#endif
            if (config.meter.voltage != volts_response->volts) {
                config.meter.voltage = volts_response->volts;
                pv_changed = true;
                voltage_notify = NOTIFY_MAX;
                save_config = true;
            }

#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            printf("volts: %u,%02u\r\n", volts_response->volts/100, volts_response->volts%100);
#endif
        }
    }
}

void get_power_data() {

    power_meter_data_t  *power_response;

    send_command(cmd_power_data);
    sleep_ms(200);
    if (request_pkt.pkt_len > 0) {
        response_meter(cmd_power_data);
        if (response_pkt.pkt_len == sizeof(power_meter_data_t)) {
            power_response = (power_meter_data_t*)&response_pkt;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            uint8_t *data = (uint8_t*)&response_pkt;
            printf("response power: 0x");
            for (int i = 0; i < response_pkt.pkt_len; i++) {
                printf("%02x", data[i]);
            }
            printf("\r\n");
#endif
            if (config.meter.power != power_response->power) {
                config.meter.power = power_response->power;
                pv_changed = true;
                power_notify = NOTIFY_MAX;
                save_config = true;
            }

#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            printf("power: %u,%02u\r\n", power_response->power/100, power_response->power%100);
#endif
        }
    }

}

_attribute_ram_code_ void measure_meter() {

    if (first_start_data()) {
        get_current_data();
        get_voltage_data();
        get_power_data();
    }
    if (save_config) {
        write_config();
    }

}



#endif

