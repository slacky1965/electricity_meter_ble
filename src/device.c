#include <stdint.h>
#include "tl_common.h"
#include "stack/ble/ble.h"

#include "device.h"

_attribute_data_retention_ uint8_t  tariff_changed = true;
_attribute_data_retention_ uint8_t  pv_changed = true;
_attribute_data_retention_ meter_t meter = {0};

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


