#include <stdint.h>
#include "tl_common.h"
#include "stack/ble/ble.h"

#include "device.h"
#include "app_uart.h"
#include "cfg.h"

_attribute_data_retention_ uint8_t  tariff_changed = true;
_attribute_data_retention_ uint8_t  pva_changed = true;
_attribute_data_retention_ meter_t  meter;
_attribute_data_retention_ uint8_t  release_month;
_attribute_data_retention_ uint8_t  release_year;
_attribute_data_retention_ uint8_t  new_start = true;
_attribute_data_retention_ pkt_error_t pkt_error_no;

_attribute_ram_code_ uint16_t get_divisor(const uint8_t division_factor) {

    switch (division_factor & 0x03) {
        case 0: return 1;
        case 1: return 10;
        case 2: return 100;
        case 3: return 1000;
    }

    return 1;
}

_attribute_ram_code_ uint32_t from24to32(const uint8_t *str) {

    uint32_t value;

    value = str[0] & 0xff;
    value |= (str[1] << 8) & 0xff00;
    value |= (str[2] << 16) & 0xff0000;

    return value;
}

_attribute_ram_code_ uint8_t set_device_type(device_type_t type) {

    memset(&meter, 0, sizeof(meter_t));
    new_start = true;
    uint8_t save = false;
    uint16_t divisor;

    switch (type) {
        case device_kaskad_1_mt:
            if (config.save_data.device_type != device_kaskad_1_mt) {
                config.save_data.device_type = device_kaskad_1_mt;
                divisor = 0x0a4f;   /* power 1000, voltage 0.1, amps 0.1, tariffs 10 */
                memcpy(&config.save_data.divisor, &divisor, sizeof(divisor_t));
                write_config();
                save = true;
            }
            meter.measure_meter = measure_meter_kaskad1mt;
            meter.get_date_release_data = get_date_release_data_kaskad1mt;
            meter.get_serial_number_data = get_serial_number_data_kaskad1mt;
#if UART_PRINT_DEBUG_ENABLE
            printf("Device type KACKAD-1-MT\r\n");
#endif
            break;
        case device_kaskad_11:
            if (config.save_data.device_type != device_kaskad_11) {
                config.save_data.device_type = device_kaskad_11;
                memset(&config.save_data.divisor, 0, sizeof(divisor_t));
                write_config();
                save = true;
            }
            meter.measure_meter = measure_meter_kaskad11;
            meter.get_date_release_data = get_date_release_data_kaskad11;
            meter.get_serial_number_data = get_serial_number_data_kaskad11;
#if UART_PRINT_DEBUG_ENABLE
            printf("Device type KACKAD-11\r\n");
#endif
            break;
        case device_mercury_206:
            if (config.save_data.device_type != device_mercury_206) {
                config.save_data.device_type = device_mercury_206;
                memset(&config.save_data.divisor, 0, sizeof(divisor_t));
                write_config();
                save = true;
            }
            meter.measure_meter = measure_meter_mercury206;
            meter.get_date_release_data = get_date_release_data_mercury206;
            meter.get_serial_number_data = get_serial_number_data_mercury206;
#if UART_PRINT_DEBUG_ENABLE
            printf("Device type Mercury-206\r\n");
#endif
            break;
        case device_energomera_ce102m:
            if (config.save_data.device_type != device_energomera_ce102m) {
                config.save_data.device_type = device_energomera_ce102m;
                divisor =  0x0a09;   /* power 0.1, voltage 0.1, amps 1, tariffs 10 */
                memset(&config.save_data.divisor, 0, sizeof(divisor_t));
                write_config();
                save = true;
            }
            meter.measure_meter = measure_meter_energomera_ce102m;
            meter.get_date_release_data = get_date_release_data_energomera_ce102m;
            meter.get_serial_number_data = get_serial_number_data_energomera_ce102m;
#if UART_PRINT_DEBUG_ENABLE
            printf("Device type Energomera CE-102M\r\n");
#endif
            break;
        default:
            config.save_data.device_type = device_kaskad_1_mt;
            divisor = 0x0a4f;   /* power 1000, voltage 0.1, amps 0.1, tariffs 10 */
            memcpy(&config.save_data.divisor, &divisor, sizeof(divisor_t));
            meter.measure_meter = measure_meter_kaskad1mt;
            meter.get_date_release_data = get_date_release_data_kaskad1mt;
            meter.get_serial_number_data = get_serial_number_data_kaskad1mt;
            write_config();
            save = true;
#if UART_PRINT_DEBUG_ENABLE
            printf("Default device type KACKAD-1-MT\r\n");
#endif
            break;
    }
    return save;
}

#if UART_PRINT_DEBUG_ENABLE
_attribute_ram_code_ void print_error(pkt_error_t err_no) {

    switch (err_no) {
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
}
#endif
