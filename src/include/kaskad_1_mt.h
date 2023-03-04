#ifndef SRC_INCLUDE_KASKAD_1_MT_H_
#define SRC_INCLUDE_KASKAD_1_MT_H_

typedef enum _cmd_kaskad_1_mt_t {
    cmd_k1mt_open_channel         = 0x01,
    cmd_k1mt_tariffs_data         = 0x05,
    cmd_k1mt_read_configure       = 0x10,
    cmd_k1mt_resource_battery     = 0x1e,
    cmd_k1mt_volts_data           = 0x0129,   /* command 0x29, sub command 0x01 */
    cmd_k1mt_amps_data            = 0x012c,
    cmd_k1mt_power_data           = 0x2d,
    cmd_k1mt_serial_number        = 0x010a,
    cmd_k1mt_date_release         = 0x020a,
    cmd_k1mt_factory_manufacturer = 0x030a,
    cmd_k1mt_name_device          = 0x040a,
    cmd_k1mt_name_device2         = 0x050a,
    cmd_k1mt_get_info             = 0x30,
    cmd_k1mt_test_error           = 0x60
} cmd_kaskad_1_mt_t;

typedef struct __attribute__((packed)) _k1mt_package_header_t {
    uint8_t  data_len   :5;     /* 0-4 bits - data lenght                               */
    uint8_t  from_to    :1;     /* 1 request to the device, 0 response from the device  */
    uint8_t  cpu_power  :1;     /* 1 sufficient computing power, 0 weak computing power */
    uint8_t  crypted    :1;     /* 1 crypted, 0 non crypted                             */
    uint8_t  reserved;
    uint16_t address_to;
    uint16_t address_from;
    uint8_t  command;
    uint32_t password_status;
} k1mt_package_header_t;

typedef struct __attribute__((packed)) _k1mt_response_status_t {
    uint8_t  role;
    uint8_t  info1;
    uint8_t  info2;
    uint8_t  error;
} k1mt_response_status_t;

typedef struct __attribute__((packed)) _k1mt_package_t {
    uint8_t          start;
    uint8_t          boundary;
    k1mt_package_header_t header;
    uint8_t          data[PKT_BUFF_MAX_LEN];
    uint8_t          pkt_len;
    uint8_t          load_len;
} k1mt_package_t;

typedef struct __attribute__((packed)) _k1mt_pkt_tariffs_t {
    uint32_t         sum_tariffs;
    uint8_t          byte_cfg;
    uint8_t          division_factor;
    uint8_t          role;
    uint8_t          multiplication_factor[3];
    uint32_t         tariff_1;
    uint32_t         tariff_2;
    uint32_t         tariff_3;
    uint32_t         tariff_4;
} k1mt_pkt_tariffs_t;

typedef struct __attribute__((packed)) _k1mt_pkt_amps_t {
    uint8_t          phase_num; /* number of phase    */
    uint8_t          amps[3];   /* maybe 2 or 3 bytes */
} k1mt_pkt_amps_t;

typedef struct __attribute__((packed)) _k1mt_pkt_volts_t {
    uint8_t          phase_num;
    uint16_t         volts;
} k1mt_pkt_volts_t;

typedef struct __attribute__((packed)) _k1mt_pkt_power_t {
    uint8_t          power[3];
    uint8_t          byte_cfg;
    uint8_t          division_factor;
} k1mt_pkt_power_t;

typedef struct __attribute__((packed)) _k1mt_pkt_read_cfg_t {
    uint8_t          divisor            :2; /* 0 - "00000000", 1 - "0000000.0", 2 - "000000.00", 3 - "00000.000"    */
    uint8_t          current_tariff     :2; /* 0 - first, 1 - second, 2 - third, 3 - fourth                         */
    uint8_t          char_num           :2; /* 0 - 6, 1 - 7, 2 - 8, 3 - 8                                           */
    uint8_t          tariffs            :2; /* 0 - 1, 1 - 1+2, 2 - 1+2+3, 3 - 1+2+3+4                               */
    uint8_t          summer_winter_time :1; /* automatic switching to daylight saving time or winter time 1 - on    */
    uint8_t          tariff_schedule    :2; /* 0 - work, 1 - Sat., 2 - Sun., 3 - special                            */
    uint8_t          power_limits       :1;
    uint8_t          power_limits_emerg :1;
    uint8_t          power_limits_cmd   :1;
    uint8_t          access_cust_info   :1;
    uint8_t          reserve            :1;
    uint8_t          display_time;
    uint8_t          counter_passowrd;
    uint8_t          months_worked;         /* battery */
    uint8_t          remaining_months_work; /* battery */
    uint8_t          role;
} k1mt_pkt_read_cfg_t;

typedef struct __attribute__((packed)) _k1mt_pkt_resbat_t {
    uint8_t          lifetime;
    uint8_t          worktime;
} k1mt_pkt_resbat_t;

typedef struct __attribute__((packed)) _k1mt_pkt_info_t {
    uint8_t          id;
    uint8_t          data[24];
    uint8_t          interface1;
    uint8_t          interface2;
    uint8_t          interface3;
    uint8_t          interface4;
    uint16_t         battery_mv;
} k1mt_pkt_info_t;

typedef struct __attribute__((packed)) _k1mt_pkt_data31_t {
    uint8_t          start;
    uint8_t          boundary;
    k1mt_package_header_t header;
    uint8_t          sub_command;
    uint8_t          data[DATA_MAX_LEN];    /* data31 -> data[30] + sub_command = 31 */
    uint8_t          crc;
    uint8_t          stop;
} k1mt_pkt_data31_t;

void k1mt_measure_meter();
void k1mt_get_serial_number_data();
void k1mt_get_date_release_data();


#endif /* SRC_INCLUDE_KASKAD_1_MT_H_ */
