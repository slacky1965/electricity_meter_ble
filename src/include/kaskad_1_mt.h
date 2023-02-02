#ifndef SRC_INCLUDE_KASKAD_1_MT_H_
#define SRC_INCLUDE_KASKAD_1_MT_H_

typedef enum _command_t {
    cmd_open_channel        = 0x01,
    cmd_current_data        = 0x05,
    cmd_volts_data          = 0x29,
    cmd_amps_data           = 0x2c,
    cmd_power_data          = 0x2d
} command_t;

typedef struct __attribute__((packed)) _package_header_t {
    uint8_t  params_len;        /* bit 7 6 5 4 3 2 1 0  params_len                                          */
    uint8_t  reserved;          /*     | | | | | | | |                                                      */
    uint16_t address_to;        /*     | | | --------- 0-4 bits - data lenght. 0x1f                         */
    uint16_t address_from;      /*     | | ----------- 1 request to the device, 0 response from the device  */
    uint8_t  command;           /*     | ------------- 1 sufficient computing power, 0 weak computing power */
    uint32_t password_status;   /*     --------------- 1 crypted, 0 non crypted                             */
} package_header_t;


typedef struct __attribute__((packed)) _package_t {
    uint8_t          start;
    uint8_t          boundary;
    package_header_t header;
    uint8_t          data[64];
    uint8_t          pkt_len;
    uint8_t          load_len;
} package_t;

typedef struct __attribute__((packed)) _first_meter_data_t {
    uint8_t          start;
    uint8_t          boundary;
    package_header_t header;
    uint16_t         version;
    uint16_t         addr;
    uint8_t          crc;
    uint8_t          stop;
} first_meter_data_t;

typedef struct __attribute__((packed)) _current_meter_data_t {
    uint8_t          start;
    uint8_t          boundary;
    package_header_t header;
    uint32_t         sum;
    uint32_t         counter;
    uint16_t         xz;
    uint32_t         tariff_1;
    uint32_t         tariff_2;
    uint32_t         tariff_3;
    uint32_t         tariff_4;
    uint8_t          crc;
    uint8_t          stop;
} current_meter_data_t;

typedef struct __attribute__((packed)) _amps_meter_data_t {
    uint8_t          start;
    uint8_t          boundary;
    package_header_t header;
    uint8_t          sub_cmd;
    uint16_t         amps;
    uint8_t          xz;
    uint8_t          crc;
    uint8_t          stop;
} amps_meter_data_t;

typedef struct __attribute__((packed)) _volts_meter_data_t {
    uint8_t          start;
    uint8_t          boundary;
    package_header_t header;
    uint8_t          sub_cmd;
    uint16_t         volts;
    uint8_t          crc;
    uint8_t          stop;
} volts_meter_data_t;

typedef struct __attribute__((packed)) _power_meter_data_t {
    uint8_t          start;
    uint8_t          boundary;
    package_header_t header;
    uint16_t         power;
    uint8_t          xz[3];
    uint8_t          crc;
    uint8_t          stop;
} power_meter_data_t;

#endif /* SRC_INCLUDE_KASKAD_1_MT_H_ */
