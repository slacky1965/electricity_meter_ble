#ifndef SRC_INCLUDE_DEVICE_H_
#define SRC_INCLUDE_DEVICE_H_

#define PKT_BUFF_MAX_LEN     128        /* max len read from uart          */
#define DATA_MAX_LEN         30         /* do not change!                  */

#if (ELECTRICITY_TYPE == KASKAD_1_MT)
#include "kaskad_1_mt.h"
#elif (ELECTRICITY_TYPE == KASKAD_11)
#include "kaskad_11.h"
#endif

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
    uint32_t tariff_1;                     /* last value of tariff #1            */
    uint32_t tariff_2;                     /* last value of tariff #2            */
    uint32_t tariff_3;                     /* last value of tariff #3            */
    uint32_t power;                        /* last value of power                */
    uint16_t voltage;                      /* last value of voltage              */
    uint8_t  serial_number[DATA_MAX_LEN];  /* serial number                      */
    uint8_t  serial_number_len;            /* lenght of serial number            */
    uint8_t  date_release[DATA_MAX_LEN];   /* date of release                    */
    uint8_t  date_release_len;             /* lenght of release date             */
    uint8_t  division_factor;              /* 00-0, 01-0.0, 10-0.00, 11-0.000    */
    uint8_t  battery_level;
} meter_t;

extern uint8_t tariff_changed;
extern uint8_t pv_changed;
extern meter_t meter;

void measure_meter();
uint16_t divisor(const uint8_t division_factor);
uint32_t from24to32(const uint8_t *str);

#endif /* SRC_INCLUDE_DEVICE_H_ */
