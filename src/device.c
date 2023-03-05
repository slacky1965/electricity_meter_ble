#include <stdint.h>
#include "tl_common.h"
#include "stack/ble/ble.h"

#include "device.h"
#include "mercury_206.h"
#include "cfg.h"

_attribute_data_retention_ uint8_t  tariff_changed = true;
_attribute_data_retention_ uint8_t  pv_changed = true;
_attribute_data_retention_ meter_t  meter = {0};
_attribute_data_retention_ uint8_t  release_month;
_attribute_data_retention_ uint8_t  release_year;
_attribute_data_retention_ uint8_t  new_start = true;
_attribute_data_retention_ pkt_error_t pkt_error_no;

_attribute_ram_code_ uint16_t divisor(const uint8_t division_factor) {

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

_attribute_ram_code_ void set_device_type(device_type_t type) {

    new_start = true;

    switch (type) {
        case device_kaskad_1_mt:
            config.save_data.device_type = device_kaskad_1_mt;
            meter.measure_meter = measure_meter_kaskad1mt;
            meter.get_date_release_data = get_date_release_data_kaskad1mt;
            meter.get_serial_number_data = get_serial_number_data_kaskad1mt;
            break;
        case device_kaskad_11:
            config.save_data.device_type = device_kaskad_11;
            meter.measure_meter = measure_meter_kaskad11;
            meter.get_date_release_data = get_date_release_data_kaskad11;
            meter.get_serial_number_data = get_serial_number_data_kaskad11;
            break;
        case device_mercury_206:
            config.save_data.device_type = device_mercury_206;
            meter.measure_meter = m206_measure_meter;
            meter.get_date_release_data = m206_get_date_release_data;
            meter.get_serial_number_data = m206_get_serial_number_data;
            break;
        default:
            config.save_data.device_type = device_kaskad_1_mt;
            meter.measure_meter = measure_meter_kaskad1mt;
            meter.get_date_release_data = get_date_release_data_kaskad1mt;
            meter.get_serial_number_data = get_serial_number_data_kaskad1mt;
            break;
    }
}
