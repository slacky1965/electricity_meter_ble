#ifndef SRC_INCLUDE_BLE_H_
#define SRC_INCLUDE_BLE_H_

#include "bthome.h"

#define ADV_BUFF_SIZE (31-8)

typedef enum {
    conn_disconnect         = 0x0000,
    conn_connect            = 0x0001,
    conn_delayed_disconnect = 0x0010,
    conn_delayed_reset      = 0x0100
} conn_t;

typedef struct __attribute__((packed)) _adv_head_uuid16_t {
    uint8_t  size;                  /* sum sizes adv_head_uuid16_t-1+adv_data   */
    uint8_t  type;                  /* 0x16, 16-bit UUID                        */
    uint16_t UUID;                  /* 0xFCD2, GATT Service BTHome              */
    uint8_t  device_info;           /* 0x40 or 0x41                             */
} adv_head_uuid16_t;

typedef struct __attribute__((packed)) _adv_tariff_t {
    uint8_t             flg_size;   /* 0x02                   */
    uint8_t             flg_type;   /* 0x01                   */
    uint8_t             flg;        /* 0x06                   */
    adv_head_uuid16_t   head;
    tariff_t            tariff;
} adv_tariff_t;

typedef struct __attribute__((packed)) _adv_crypt_tariff_t {
    uint8_t             flg_size;   /* 0x02                   */
    uint8_t             flg_type;   /* 0x01                   */
    uint8_t             flg;        /* 0x06                   */
    adv_head_uuid16_t   head;
    uint8_t             ciphertext[sizeof(tariff_t)];
    uint32_t            counter;
    uint8_t             mic[4];
} adv_crypt_tariff_t;

typedef struct __attribute__((packed)) _adv_power_voltage_t {
    uint8_t             flg_size;   /* 0x02                   */
    uint8_t             flg_type;   /* 0x01                   */
    uint8_t             flg;        /* 0x06                   */
    adv_head_uuid16_t   head;
    power_voltage_t     pv;
} adv_power_voltage_t;

typedef struct __attribute__((packed)) _adv_crypt_power_voltage_t {
    uint8_t             flg_size;   /* 0x02                   */
    uint8_t             flg_type;   /* 0x01                   */
    uint8_t             flg;        /* 0x06                   */
    adv_head_uuid16_t   head;
    uint8_t             ciphertext[sizeof(power_voltage_t)];
    uint32_t            counter;
    uint8_t             mic[4];
} adv_crypt_power_voltage_t;

typedef struct __attribute__((packed)) _main_notify_t {
    uint16_t id;
    uint16_t measurement_period;
    uint8_t  version;
    uint8_t  encrypted;
    uint32_t address;
} main_notify_t;

typedef struct __attribute__((packed)) _bindkey_notify_t {
    uint16_t id;
    uint8_t  bindkey[16];
} bindkey_notify_t;

typedef struct __attribute__((packed)) _serial_number_notify_t {
    uint16_t id;
    uint8_t  serial_number[16];
} serial_number_notify_t;

typedef struct __attribute__((packed)) _date_release_notify_t {
    uint16_t id;
    uint8_t  date_release[16];
} date_release_notify_t;


extern uint16_t ble_connected;
extern uint8_t ota_is_working;
extern uint8_t mac_public[6], mac_random_static[6];
extern uint8_t ble_name[BLE_NAME_SIZE];
extern adv_tariff_t adv_tariff_data;
extern adv_power_voltage_t adv_pv_data;
extern adv_crypt_tariff_t adv_crypt_tariff_data;
extern adv_crypt_power_voltage_t adv_crypt_pv_data;
extern main_notify_t main_notify;
extern bindkey_notify_t bindkey_notify;
extern serial_number_notify_t serial_number_notify;
extern date_release_notify_t date_release_notify;
extern uint8_t tariff_changed;
extern uint8_t pv_changed;
extern uint8_t adv_counter;



void init_ble();
void set_adv_data(uint8_t *adv_data, uint8_t data_size);
void ev_adv_timeout(u8 e, u8 *p, int n);
void ble_send_tariff1();
void ble_send_tariff2();
void ble_send_tariff3();
void ble_send_tariff4();
void ble_send_power();
void ble_send_voltage();
void ble_send_main();
void ble_send_log();
void ble_send_bindkey();
void ble_send_serila_number();
void ble_send_date_release();


#endif /* SRC_INCLUDE_BLE_H_ */
