#ifndef SRC_INCLUDE_KASKAD_1_MT_H_
#define SRC_INCLUDE_KASKAD_1_MT_H_

#define PKT_BUFF_MAX_LEN     128        /* max len read from uart          */

typedef enum _command_t {
    cmd_open_channel         = 0x01,
    cmd_current_data         = 0x05,
    cmd_volts_data           = 0x0129,   /* command 0x29, sub command 0x01 */
    cmd_amps_data            = 0x012c,
    cmd_power_data           = 0x2d,
    cmd_serial_number        = 0x010a,
    cmd_date_release         = 0x020a,
    cmd_factory_manufacturer = 0x030a,
    cmd_name_device          = 0x040a,
    cmd_name_device2         = 0x050a
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
    uint8_t          data[PKT_BUFF_MAX_LEN];
    uint8_t          pkt_len;
    uint8_t          load_len;
} package_t;

typedef struct __attribute__((packed)) _tariffs_meter_data_t {
    uint32_t         sum_tariffs;
    uint8_t          byte_cfg;
    uint8_t          division_factor;
    uint8_t          role;
    uint8_t          multiplication_factor[3];
    uint32_t         tariff_1;
    uint32_t         tariff_2;
    uint32_t         tariff_3;
    uint32_t         tariff_4;
} tariffs_meter_data_t;

typedef struct __attribute__((packed)) _amps_meter_data_t {
    uint8_t          phase_num; /* number of phase    */
    uint8_t          amps[3];   /* value 2 or 3 bytes */
} amps_meter_data_t;

typedef struct __attribute__((packed)) _volts_meter_data_t {
    uint8_t          phase_num;
    uint16_t         volts;
} volts_meter_data_t;

typedef struct __attribute__((packed)) _power_meter_data_t {
    uint8_t          power[3];
    uint8_t          byte_cfg;
    uint8_t          division_factor;
} power_meter_data_t;

typedef struct __attribute__((packed)) _data31_meter_data_t {
    uint8_t          start;
    uint8_t          boundary;
    package_header_t header;
    uint8_t          sub_command;
    uint8_t          data[30];      /* data31 -> data[30] + sub_command = 31 */
    uint8_t          crc;
    uint8_t          stop;
} data31_meter_data_t;

void get_serial_number_data();
void get_date_release_data();


#endif /* SRC_INCLUDE_KASKAD_1_MT_H_ */
