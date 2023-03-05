#ifndef SRC_INCLUDE_DEVICE_H_
#define SRC_INCLUDE_DEVICE_H_

#define PKT_BUFF_MAX_LEN     128        /* max len read from uart          */
#define DATA_MAX_LEN         30         /* do not change!                  */

typedef enum _device_type_t {
    device_undefined = 0,
    device_kaskad_1_mt,
    device_kaskad_11,
    device_mercury_206
} device_type_t;

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
    uint32_t tariff_1;                      /* last value of tariff #1            */
    uint32_t tariff_2;                      /* last value of tariff #2            */
    uint32_t tariff_3;                      /* last value of tariff #3            */
    uint32_t power;                         /* last value of power                */
    uint16_t voltage;                       /* last value of voltage              */
    uint32_t amps;                          /* last valie of ampere               */
    uint8_t  serial_number[DATA_MAX_LEN+1]; /* serial number                      */
    uint8_t  serial_number_len;             /* lenght of serial number            */
    uint8_t  date_release[DATA_MAX_LEN+1];  /* date of release                    */
    uint8_t  date_release_len;              /* lenght of release date             */
    uint8_t  division_factor;               /* 00-0, 01-0.0, 10-0.00, 11-0.000    */
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

uint16_t divisor(const uint8_t division_factor);
uint32_t from24to32(const uint8_t *str);
void set_device_type(device_type_t type);

void measure_meter_kaskad1mt();
void get_serial_number_data_kaskad1mt();
void get_date_release_data_kaskad1mt();
void measure_meter_kaskad11();
void get_serial_number_data_kaskad11();
void get_date_release_data_kaskad11();
void measure_meter_mercury206();
void get_serial_number_data_mercury206();
void get_date_release_data_mercury206();

#endif /* SRC_INCLUDE_DEVICE_H_ */
