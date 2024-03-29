#ifndef SRC_INCLUDE_DEVICE_H_
#define SRC_INCLUDE_DEVICE_H_

#define PKT_BUFF_MAX_LEN    128         /* max len read from uart          */
#define DATA_MAX_LEN        30          /* do not change!                  */
#define MULTIPLIER          1
#define DIVISOR             0

#define RESOURCE_BATTERY    120         /* in month */

typedef enum _device_type_t {
    device_undefined = 0,
    device_kaskad_1_mt,
    device_kaskad_11,
    device_mercury_206,
    device_energomera_ce102m,
} device_type_t;

typedef struct __attribute__((packed)) _divisor_t {
    uint16_t power_divisor      :2;     /* 00-0, 01-10, 10-100, 11-1000     */
    uint16_t power_sign         :1;     /* 0 - divisor, 1 - multiplier      */
    uint16_t voltage_divisor    :2;
    uint16_t voltage_sign       :1;
    uint16_t current_divisor    :2;
    uint16_t current_sign       :1;
    uint16_t tariffs_divisor    :2;
    uint16_t tariffs_sign       :1;
    uint16_t reserve            :4;
} divisor_t;

typedef enum _pkt_error_t {
    PKT_OK  = 0,
    PKT_ERR_NO_PKT,
    PKT_ERR_TIMEOUT,
    PKT_ERR_UNKNOWN_FORMAT,
    PKT_ERR_DIFFERENT_COMMAND,
    PKT_ERR_INCOMPLETE,
    PKT_ERR_UNSTUFFING,
    PKT_ERR_ADDRESS,
    PKT_ERR_RESPONSE,
    PKT_ERR_CRC,
    PKT_ERR_UART
} pkt_error_t;

typedef struct __attribute__((packed)) _meter_t {
    uint32_t tariff_1;                      /* last value of tariff #1      */
    uint32_t tariff_2;                      /* last value of tariff #2      */
    uint32_t tariff_3;                      /* last value of tariff #3      */
    uint32_t power;                         /* last value of power          */
    uint16_t voltage;                       /* last value of voltage        */
    uint16_t amps;                          /* last value of ampere         */
    uint8_t  serial_number[DATA_MAX_LEN+1]; /* serial number                */
    uint8_t  serial_number_len;             /* lenght of serial number      */
    uint8_t  date_release[DATA_MAX_LEN+1];  /* date of release              */
    uint8_t  date_release_len;              /* lenght of release date       */
    uint8_t  battery_level;
    void   (*measure_meter) (void);
    void   (*get_serial_number_data) (void);
    void   (*get_date_release_data) (void);
} meter_t;

extern uint8_t tariff_changed;
extern uint8_t pva_changed;
extern meter_t meter;
extern uint8_t release_month;
extern uint8_t release_year;
extern uint8_t new_start;
extern pkt_error_t pkt_error_no;

uint16_t get_divisor(const uint8_t division_factor);
uint32_t from24to32(const uint8_t *str);
uint8_t set_device_type(device_type_t type);
void print_error(pkt_error_t err_no);

void measure_meter_kaskad1mt();
void get_serial_number_data_kaskad1mt();
void get_date_release_data_kaskad1mt();
void measure_meter_kaskad11();
void get_serial_number_data_kaskad11();
void get_date_release_data_kaskad11();
void measure_meter_mercury206();
void get_serial_number_data_mercury206();
void get_date_release_data_mercury206();
void measure_meter_energomera_ce102m();
void get_serial_number_data_energomera_ce102m();
void get_date_release_data_energomera_ce102m();
void measure_meter_energomera_ce102();

#endif /* SRC_INCLUDE_DEVICE_H_ */
