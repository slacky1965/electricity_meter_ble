#ifndef SRC_INCLUDE_CFG_H_
#define SRC_INCLUDE_CFG_H_

#define ID_CONFIG   0x0FED141A
#define ID_METER    0x0FED1978
#define TOP_MASK    0xFFFFFFFF

#include "device.h"

#define MEASUREMENT_PERIOD 1                /* in minutes                         */

typedef struct __attribute__((packed)) _meter_t {
    uint32_t id;                            /* ID - ID_METER                      */
    uint32_t tariff_1;                      /* last value of tariff #1            */
    uint32_t tariff_2;                      /* last value of tariff #2            */
    uint32_t tariff_3;                      /* last value of tariff #3            */
    uint32_t power;                         /* last value of power                */
    uint16_t voltage;                       /* last value of voltage              */
    uint16_t address;                       /* address of meter                   */
    uint8_t  serial_number[DATA_MAX_LEN];   /* serial number                      */
    uint8_t  sn_len;                        /* lenght of serial number            */
    uint8_t  date_release[DATA_MAX_LEN];    /* date of release                    */
    uint8_t  dr_len;                        /* lenght of release date             */
    uint8_t  division_factor;               /* 00-0, 01-0.0, 10-0.00, 11-0.000    */
    uint16_t battery_mv;
//    uint8_t  test;
} meter_t;

/* must be no more than FLASH_PAGE_SIZE (256) bytes */
typedef struct __attribute__((packed)) _config_t {
    uint32_t id;                            /* 1st place only! ID - ID_CONFIG     */
    uint32_t top;                           /* 2st place only! 0x0 .. 0xFFFFFFFF  */
    uint32_t flash_addr;                    /* 3st place only! flash page address */
    meter_t  meter;                         /* 4st place only! meter values       */
    uint8_t  size;                          /* 5st place only! sizeof config      */
    uint16_t measurement_period;            /* measurement period in sec.         */
    uint8_t  ble_name[BLE_NAME_SIZE];       /* Module name with MAC               */
    uint8_t  encrypted;                     /* non-encrypted = 0, encrypted = 1   */
    uint8_t  bindkey[16];                   /* secret key                         */
} config_t;

extern config_t config;
extern uint8_t save_config;

void init_config();
void write_config();
void clear_config();                        /* clear config.meter only!!! */


#endif /* SRC_INCLUDE_CFG_H_ */
