#ifndef SRC_INCLUDE_KASKAD_1_MT_H_
#define SRC_INCLUDE_KASKAD_1_MT_H_

typedef enum _cmd_t {
    cmd_open_channel        = 0x01,
    cmd_current_data        = 0x05,
    cmd_volts_data          = 0x29,
    cmd_amps_data           = 0x2c,
    cmd_power_data          = 0x2d,
} cmd_t;

typedef struct __attribute__((packed)) _request_header_t {
    uint8_t             start;      /* 0x73 */
    uint8_t             boundary;   /* 0x55 */
    uint8_t             add_len;    /* 0x20 - 15 bytes or 0x21 - 16 bytes. add_len & 0x0f */
    uint8_t             nothing;    /* 0x00 */
    uint16_t            meter_address;
    uint16_t            conn_type;  /* 0xffff for optoport */
    uint8_t             command;
    uint32_t            password;   /* 0x00000000   */
} request_header_t;

typedef struct __attribute__((packed)) _request_pkt_t {
    request_header_t    head;
    uint8_t             tail[16];    /* 2 bytes - crc+boundary or 3 bytes - add_cmd+crc+boundary */
    uint8_t             pkt_len;
    uint8_t             load_len;
} request_pkt_t;

typedef struct __attribute__((packed)) _response_header_t {
    uint8_t             start;      /* 0x73 */
    uint8_t             boundary;   /* 0x55 */
    uint8_t             data_len;   /* without start,begin boundary,crc, end boundary */
    uint8_t             not;        /* 0x00 */
    uint16_t            conn_type;  /* 0xffff for optoport */
    uint16_t            meter_address;
    uint8_t             command;
} response_header_t;

typedef struct __attribute__((packed)) _response_pkt_t {
    response_header_t   head;
    uint8_t             response_data[64];  /* data_len+crc+end boundary */
    uint8_t             pkt_len;
} response_pkt_t;

typedef struct __attribute__((packed)) _first_meter_data_t {
    response_header_t   head;
    uint16_t            version;
    uint16_t            addr;
    uint8_t             crc;
    uint8_t             boundary;
} first_meter_data_t;

typedef struct __attribute__((packed)) _current_meter_data_t {
    response_header_t   head;
    uint32_t            sum;
    uint32_t            counter;
    uint16_t            xz;
    uint32_t            tariff_1;
    uint32_t            tariff_2;
    uint32_t            tariff_3;
    uint32_t            tariff_4;
    uint8_t             crc;
    uint8_t             boundary;
} current_meter_data_t;

typedef struct __attribute__((packed)) _amps_meter_data_t {
    response_header_t   head;
    uint8_t             sub_cmd;
    uint16_t            amps;
    uint8_t             xz;
    uint8_t             crc;
    uint8_t             boundary;
} amps_meter_data_t;

typedef struct __attribute__((packed)) _volts_meter_data_t {
    response_header_t   head;
    uint8_t             sub_cmd;
    uint16_t            volts;
    uint8_t             crc;
    uint8_t             boundary;
} volts_meter_data_t;

typedef struct __attribute__((packed)) _power_meter_data_t {
    response_header_t   head;
    uint16_t            power;
    uint8_t             xz[3];
    uint8_t             crc;
    uint8_t             boundary;
} power_meter_data_t;

extern request_pkt_t request_pkt;
extern response_pkt_t response_pkt;

#endif /* SRC_INCLUDE_KASKAD_1_MT_H_ */
