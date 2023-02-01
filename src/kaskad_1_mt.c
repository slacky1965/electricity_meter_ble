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
_attribute_data_retention_ static pkt_err_t pkt_err_no;

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

    uint8_t len = 0;

    set_command(command);

    for (uint8_t attempt = 0; attempt < 3; attempt++) {
        len = send_to_uart((uint8_t*)&request_pkt, request_pkt.pkt_len);
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
            break;
        } else {
            request_pkt.load_len = 0;
        }
    }

    if (request_pkt.load_len == 0) {
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        printf("Can't send a request pkt\r\n");
#endif
    }
}

static pkt_err_t response_meter(cmd_t command) {

    uint8_t buff[UART_DATA_LEN] = {0};
    uint8_t data_len, len, load_size = 0;
    pkt_err_no = PKT_OK;

    memset(&response_pkt, 0, sizeof(response_pkt));



    for (uint8_t attempt = 0; attempt < 3; attempt ++) {
        data_len = get_data_len_from_uart();
        load_size = 0;
        while (1) {
            len = response_from_uart(buff+load_size, data_len-load_size);
            if (len == -1) {
                pkt_err_no =  PKT_ERR_UART;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
                printf("Attempt to read data from uart: %u\r\n", attempt+1);
#endif
                break;
            } else if (len == 0) {
                if (load_size == 0) {
                    pkt_err_no = PKT_ERR_TIMEOUT;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
                    printf("Attempt to read data from uart: %u\r\n", attempt+1);
#endif
                    break;
                }
                attempt = 3;
                break;
            } else if (len < data_len-load_size) {
                load_size += len;
            } else {
                load_size += len;
                attempt = 3;
                break;
            }
        }
        sleep_ms(100);
    }

    if (load_size && load_size == data_len) {
        if (*buff == START && *(buff+1) == BOUNDARY && *(buff+load_size-1) == BOUNDARY && *(buff+8) == command) {
            memcpy(&response_pkt, buff, sizeof(response_header_t));
            len = response_pkt.head.data_len+2;
            memcpy(response_pkt.response_data, buff+(load_size-len), len);
            response_pkt.pkt_len = sizeof(response_header_t)+len;
            pkt_err_no = PKT_OK;
        }
    }

    return pkt_err_no;
}

static response_pkt_t *get_pkt_data(cmd_t command) {

    send_command(command);
    sleep_ms(200);
    if (request_pkt.pkt_len > 0) {
        if (response_meter(command) == PKT_OK) {
            return &response_pkt;
        }
    }
    return NULL;
}

uint8_t first_start_data() {

    first_meter_data_t *first_reponse;
    response_pkt_t     *pkt;

    pkt = get_pkt_data(cmd_open_channel);

    if (pkt) {
        first_reponse = (first_meter_data_t*)pkt;
        if (first_reponse->addr == config.meter.address) {
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
            uint8_t *data = (uint8_t*)pkt;
            printf("response first start: 0x");
            for (int i = 0; i < response_pkt.pkt_len; i++) {
                printf("%02x", data[i]);
            }
            printf("\r\n");
#endif
            return true;
        }
    }

    return false;
}

void get_current_data() {

    current_meter_data_t *current_response;
    response_pkt_t       *pkt;

    pkt = get_pkt_data(cmd_current_data);

    if (pkt) {
        current_response = (current_meter_data_t*)pkt;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        uint8_t *data = (uint8_t*)pkt;
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

void get_amps_data() {

    amps_meter_data_t *amps_response;
    response_pkt_t    *pkt;

    pkt = get_pkt_data(cmd_amps_data);

    if (pkt) {
        amps_response = (amps_meter_data_t*)pkt;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        uint8_t *data = (uint8_t*)pkt;
        printf("response amps: 0x");
        for (int i = 0; i < response_pkt.pkt_len; i++) {
            printf("%02x", data[i]);
        }
        printf("\r\n");
        printf("amps: %u,%02u\r\n", amps_response->amps/1000, amps_response->amps%1000);
#endif
    }
}

void get_voltage_data() {

    volts_meter_data_t *volts_response;
    response_pkt_t     *pkt;

    pkt = get_pkt_data(cmd_volts_data);

    if (pkt) {
        volts_response = (volts_meter_data_t*)pkt;
#if UART_PRINT_DEBUG_ENABLE && UART_DEBUG
        uint8_t *data = (uint8_t*)pkt;
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

