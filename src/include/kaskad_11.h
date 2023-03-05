#ifndef SRC_INCLUDE_KASKAD_11_H_
#define SRC_INCLUDE_KASKAD_11_H_

typedef enum _cmd_kaskad_11_t {
    cmd_k11_open_channel    = 0x02,
    cmd_k11_close_channel   = 0x03,
    cmd_k11_net_parameters  = 0x20,
    cmd_k11_serial_number   = 0x25,
    cmd_k11_tariffs_data    = 0x26,
    cmd_k11_date_release    = 0x53,
    cmd_k11_test_error      = 0xf0
} cmd_kaskad_11_t;

typedef enum _k11_net_parameters_t {
    net_voltage = 0,
    net_amps    = 1,
    net_power   = 3
} k11_net_parameters_t;

typedef struct __attribute__((packed)) _k11_package_t {
    uint8_t  len;
    uint8_t  cmd;
    uint16_t address;
    uint8_t  data[DATA_MAX_LEN];
} k11_package_t;

typedef struct __attribute__((packed)) _k11_pkt_release_t {
    uint8_t  len;
    uint8_t  cmd;
    uint16_t address;
    uint8_t  year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  status;
    uint8_t  crc;
} k11_pkt_release_t;

typedef struct __attribute__((packed)) _k11_pkt_tariff_t {
    uint8_t  len;
    uint8_t  cmd;
    uint16_t address;
    uint8_t  tariff_num;
    uint32_t value;
    uint8_t  status;
    uint8_t  crc;
} k11_pkt_tariff_t;

typedef struct __attribute__((packed)) _k11_pkt_voltage_t {
    uint8_t  len;
    uint8_t  cmd;
    uint16_t address;
    uint8_t  param;
    uint16_t voltage;
    uint8_t  status;
    uint8_t  crc;
} k11_pkt_voltage_t;

typedef struct __attribute__((packed)) _k11_pkt_voltage3_t {
    uint8_t  len;
    uint8_t  cmd;
    uint16_t address;
    uint8_t  param;
    uint16_t voltageA;
    uint16_t voltageB;
    uint16_t voltageC;
    uint8_t  status;
    uint8_t  crc;
} k11_pkt_voltage3_t;

typedef struct __attribute__((packed)) _k11_pkt_amps_t {
    uint8_t  len;
    uint8_t  cmd;
    uint16_t address;
    uint8_t  param;
    uint16_t amps;
    uint8_t  status;
    uint8_t  crc;
} k11_pkt_amps_t;

typedef struct __attribute__((packed)) _k11_pkt_amps3_t {
    uint8_t  len;
    uint8_t  cmd;
    uint16_t address;
    uint8_t  param;
    uint8_t  ampsA[3];
    uint8_t  ampsB[3];
    uint8_t  ampsC[3];
    uint8_t  status;
    uint8_t  crc;
} k11_pkt_amps3_t;

typedef struct __attribute__((packed)) _k11_pkt_power_t {
    uint8_t  len;
    uint8_t  cmd;
    uint16_t address;
    uint8_t  param;
    uint8_t  power[3];
    uint8_t  status;
    uint8_t  crc;
} k11_pkt_power_t;

typedef struct __attribute__((packed)) _k11_pkt_power3_t {
    uint8_t  len;
    uint8_t  cmd;
    uint16_t address;
    uint8_t  param;
    uint8_t  powerA[3];
    uint8_t  powerB[3];
    uint8_t  powerC[3];
    uint8_t  status;
    uint8_t  crc;
} k11_pkt_power3_t;

void k11_measure_meter();
uint8_t k11_get_serial_number_data();
void k11_get_date_release_data();

#endif /* SRC_INCLUDE_KASKAD_11_H_ */
