#ifndef SRC_INCLUDE_MERCURY_206_H_
#define SRC_INCLUDE_MERCURY_206_H_

typedef enum _cmd_mercury_206_t {
    cmd_m206_serial_number  = 0x2f,
    cmd_m206_tariffs_data   = 0x27,
    cmd_m206_net_params     = 0x63,
    cmd_m206_date_release   = 0x66,
    cmd_m206_running_time   = 0x69,
    cmd_m206_test_error     = 0xf0
} cmd_mercury_206_t;

typedef struct __attribute__((packed)) _m206_package_t {
    uint32_t address;
    uint8_t  cmd;
    uint8_t  data[DATA_MAX_LEN];
    uint8_t  pkt_len;
} m206_package_t;

typedef struct __attribute__((packed)) _m206_pkt_serial_num_t {
    uint32_t address;
    uint8_t  cmd;
    uint32_t addr;
    uint16_t crc;
} m206_pkt_serial_num_t;

typedef struct __attribute__((packed)) _m206_pkt_release_t {
    uint32_t address;
    uint8_t  cmd;
    uint8_t  date[3];
    uint16_t crc;
} m206_pkt_release_t;

typedef struct __attribute__((packed)) _m206_pkt_net_params_t {
    uint32_t address;
    uint8_t  cmd;
    uint16_t volts;
    uint16_t amps;
    uint8_t  power[3];
    uint16_t crc;
} m206_pkt_net_params_t;

typedef struct __attribute__((packed)) _m206_pkt_tariffs_t {
    uint32_t address;
    uint8_t  cmd;
    uint32_t tariff_1;
    uint32_t tariff_2;
    uint32_t tariff_3;
    uint32_t tariff_4;
    uint16_t crc;
} m206_pkt_tariffs_t;

typedef struct __attribute__((packed)) _m206_pkt_running_time_t {
    uint32_t address;
    uint8_t  cmd;
    uint8_t  tl[3];
    uint8_t  tlb[3];
    uint16_t crc;
} m206_pkt_running_time_t;

void m206_measure_meter();
uint8_t m206_get_serial_number_data();
void m206_get_date_release_data();

#endif /* SRC_INCLUDE_MERCURY_206_H_ */
