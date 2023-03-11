#ifndef SRC_INCLUDE_CFG_H_
#define SRC_INCLUDE_CFG_H_

#define ID_CONFIG   0x0FED141A
#define ID_DATA     0x0FED0978
#define TOP_MASK    0xFFFFFFFF

#include "device.h"

#define MEASUREMENT_PERIOD 1                /* in minutes                           */

typedef struct __attribute__((packed)) _save_data_t {
    uint32_t    id;                         /* id - ID_DATA                         */
    uint8_t     device_type;                /* manufacturer of electric meters      */
    uint32_t    address_device;             /* see address on dislpay ID-20109      */
    uint8_t     bindkey[16];                /* secret key                           */
    uint8_t     encrypted;                  /* non-encrypted = 0, encrypted = 1     */
    divisor_t   divisor;                    /* see device.h                         */
} save_data_t;

/* must be no more than FLASH_PAGE_SIZE (256) bytes */
typedef struct __attribute__((packed)) _config_t {
    uint32_t    id;                         /* 1st place only! ID - ID_CONFIG       */
    uint32_t    top;                        /* 2st place only! 0x0 .. 0xFFFFFFFF    */
    uint32_t    flash_addr;                 /* 3st place only! flash page address   */
    save_data_t save_data;                  /* 4st place only! save data            */
    uint8_t     size;                       /* 5st place only! sizeof config        */
    uint16_t    measurement_period;         /* measurement period in sec.           */
    uint8_t     ble_name[BLE_NAME_SIZE];    /* Module name with MAC                 */
} config_t;

extern config_t config;
extern uint8_t save_config;

void init_config();
void write_config();
void clear_config();                        /* clear config.save_data and meter     */


#endif /* SRC_INCLUDE_CFG_H_ */
